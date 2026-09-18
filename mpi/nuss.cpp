#include <cstring>
#include <iostream>
#include <vector>
#include <omp.h>
#include <iomanip>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include "hwy/highway.h"

// mpiicpx nuss_mpi.cpp -qopenmp -O3 -xHost -qopt-report -mavx2

#define GREEN "\033[32m"
#define BLUE  "\033[94m"
#define RESET "\033[0m"

namespace hn = hwy::HWY_NAMESPACE;
//#define HIGHWAY_VEC  // bb must be divided

#define paired(a1, a2) \
(((a1) == 'A' && (a2) == 'U') || \
((a1) == 'U' && (a2) == 'A') || \
((a1) == 'G' && (a2) == 'C') || \
((a1) == 'C' && (a2) == 'G'))

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


char *seqq;

const int N = 8;
const int bb = 2;
unsigned short S[N][N];
unsigned short S_CPU[N][N];

int max_send_elems = (((N + bb - 1) / bb + 1) / 2) * bb * bb;  // maks rozmiar bufora
std::vector<unsigned short> sendbuf[2];
std::vector<unsigned short> recvbuf(max_send_elems);

using namespace std;

inline void s1(int i, int j, int k) {
    S[i][j] = max(S[i][k] + S[k+1][j], S[i][j]);
}

inline void s1(int i, int j, int k, unsigned short *s) {
    *s = max(S[i][k] + S[k+1][j], *s);
}

inline void s2(int i, int j) {
    S[i][j] = max(S[i][j], S[i+1][j-1] + paired(seqq[i], seqq[j]));
}

int main(int argc, char **argv) {

    seqq = new char[N+1];

    int i,j,k;
    // initialization
    for(i=0; i<N; i++) {
        for(j=0; j<N; j++){
            S[i][j] = 0;
            S_CPU[i][j] = 0;
        }
    }

    int rank, size;
    int start = 0; int end = -1; int edge;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Request send_req = MPI_REQUEST_NULL;
    MPI_Request recv_req = MPI_REQUEST_NULL;

    int other = 1 - rank; // pozostaly, dziala tylko dla dwoch

    const char* color = (rank == 0) ? GREEN : BLUE;

    sendbuf[0].resize(max_send_elems);
    sendbuf[1].resize(max_send_elems);

    printf("START rank=%d size=%d\n", rank, size);
    fflush(stdout);

    if (rank == 0) {
        char znaki[] = {'C', 'G', 'U', 'A'};
        srand(static_cast<unsigned short>(time(0)));
        for (short i = 0; i < N; i++)
            seqq[i] = znaki[rand() % 4];  // Losowy wybór z zestawu 'C', 'G', 'U', 'A'
        seqq[N] = '\0';
        cout << seqq << endl;
    }

    MPI_Bcast(seqq, N + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    int phalf0=0; int phalf1=0; int phalf = 0;
    int ps0, pe0, ps1, pe1;

    int last_c1 = (N-1) / bb;

    double start_time, end_time;
    start_time = omp_get_wtime();

    MPI_Barrier(MPI_COMM_WORLD);
    for (int c0 = 0; c0 <= (N - 1)/bb; c0 += 1) {
        // iRecv edge musi poczekac dwa bufory wtedy
        int odd = c0 & 1;  // % 2

        if (c0 == 0) {
            std::fill(sendbuf[odd].begin(), sendbuf[odd].end(), 0);  // trojkatne
        }

        int blocks_on_diag = last_c1 - c0 + 1;

        int half0 = blocks_on_diag / 2;
        int half1 = (blocks_on_diag + 1) / 2;
        int half;

        if (rank == 0)
            cout << "half0 = " << half0 << ", half1 = " << half1 << endl;

        int start0 = c0;
        int end0   = c0 + half0 - 1;

        int start1 = c0 + half0;
        int end1   = last_c1;

        // skrajne bloki:
        int edge0 = end0;    // ostatni c1 pierwszej połowy
        int edge1 = start1;  // pierwszy c1 drugiej połowy

        // phalf ustawiamy na othera
        if (rank == 0) { start = start0; end = end0; edge = edge0; phalf = phalf1; half = half0; }
        if (rank == 1) { start = start1; end = end1; edge = edge1; phalf = phalf0; half = half1;}

        if (rank == 0)
            cout << "[edge0: " << end0 << ",edge1: " << start1 << "]" << endl;

        if (c0 > 0) {
            if (rank == 0) {
                cout << "0 receives " << phalf1 << " from 1 [ "<< ps1 << "-" << pe1 <<" ]" << endl;
                cout << "1 receives " << phalf0 << " from 0 [ "<< ps0 << "-" << pe0 <<" ]" << endl;
            }
        }

        if (c0 > 0) {
            int recv_count = phalf * bb * bb;
            MPI_Irecv(recvbuf.data(), recv_count, MPI_UNSIGNED_SHORT,
            other, (c0-1)+(other+1)*1000, MPI_COMM_WORLD, &recv_req);
        }



        //for (int c1 = c0; c1 <= N / bb; c1 += 1) {
        #pragma omp parallel for schedule(dynamic, 1)  // sprawdz chunksize
        for (int c1 = start; c1 <= end; c1 += 1) {

            unsigned short C[bb][bb] = {0};

            short _ii = c1-c0;
            short _jj = c1;

            int local_block_id = c1 - start;
            int base_block = local_block_id * bb * bb;

            //cout << color << "!!!" << local_block_id << RESET << endl;


            if (c1 ==edge) {
                MPI_Wait(&send_req, MPI_STATUS_IGNORE); // poczekaj na wysylke poprzedniego
                if (c0 > 0)
                    MPI_Wait(&recv_req, MPI_STATUS_IGNORE); // poczekaj na odbior poprzedniego
                // wypkuj recvbuf do S

                int recv_blocks = phalf;
                int recv_start  = (rank == 0) ? ps1 : ps0;
                int prev_c0     = c0 - 1;

                for (int b = 0; b < recv_blocks; b++) {
                    int t1 = recv_start + b;

                    int __ii = t1 - prev_c0;
                    int __jj = t1;

                    int base = b * bb * bb;

                    int i0 = bb * __ii;
                    int j0 = bb * __jj;
                    int cols = min(bb, N - j0);

                    for (int row = 0; row < bb; row++) {
                        int c2 = i0 + row;
                        std::memcpy(
                            &S[c2][j0],
                            &recvbuf[base + row * bb],
                            cols * sizeof(unsigned short)
                        );
                    }
                }


            }

                        // highway
            #ifdef HIGHWAY_VEC
            const hn::ScalableTag<uint16_t> d;
            const size_t L = hn::Lanes(d);

            for (short kk = _ii + 1; kk < _jj; kk++) {
                for (int row = 0; row < bb; row++) {
                    const int i  = bb * _ii + row;
                    const int j0 = bb * _jj;

                    for (int k = 0; k < bb; k++) {
                        const int left  = bb * kk + k - 1;
                        const int right = bb * kk + k;

                        const auto a = hn::Set(d, static_cast<uint16_t>(S[i][left]));

                        for (size_t col = 0; col < bb; col += L) {
                            //const size_t n = min(L, static_cast<size_t>(bb) - col);
                            auto c = hn::LoadU(d, &C[row][col]);
                            auto s = hn::LoadU(d, &S[right][j0 + col]);

                            c = hn::Max(c, hn::Add(a, s));

                            hn::StoreU(c, d, &C[row][col]);
                        }
                    }
                }
            }
            #else
            for (short kk = _ii + 1; kk < _jj; kk++)
                for (int row = 0; row < bb; row++)
                    #pragma omp simd
                    for (int col = 0; col < bb; col++)  // simd
                        for (int k = 0; k < bb; k++)
                               s1(bb * _ii + row, bb * _jj + col, bb * kk + k - 1, &C[row][col]);
            #endif

            if (_jj >= _ii + 1) {
                for (int c2 = bb * _ii + (bb - 1); c2 >= bb * _ii; c2--) {
                    int row = c2 - bb * _ii;
                    for (int c3 = bb * _jj; c3 <= min(N - 1, bb * _jj + (bb - 1)); c3 += 1) {

                        for (int c4 = c2; c4 < c2 + bb-1; c4 += 1)
                            s1(c2, c3, c4);

                        S[c2][c3] = max(C[c2 - bb*_ii][c3 - bb*_jj],S[c2][c3]);

                        for (int c4 = bb * _jj - 1; c4 < c3; c4 += 1)
                            s1(c2, c3, c4);
                        s2(c2, c3);


                        int col = c3 - bb * _jj;
                        int buf_idx = base_block + row * bb + col;
                        cout << color << buf_idx  << " -> " << c2 << " " << c3 << RESET << endl;
                        sendbuf[odd][buf_idx] = S[c2][c3];

                    }
                }
            }

            else if (_jj == _ii) {
                for (int c2 = min(N - 2, bb * _jj + (bb - 2)); c2 >=  bb * _jj; c2--) {
                    int row = c2 - bb * _ii;

                    for (int c3 = c2 + 1; c3 <= min(N - 1, bb * _jj + (bb - 1)); c3 += 1) {
                        for (int c4 = c2; c4 < c3; c4 += 1)
                            s1(c2, c3, c4);
                        s2(c2, c3);

                        int col = c3 - bb * _jj;
                        int buf_idx = base_block + row * bb + col;
                        cout << color << buf_idx  << " -> " << c2 << " " << c3 << RESET << endl;
                        sendbuf[odd][buf_idx] = S[c2][c3];
                    }
                }
            }

        }

        phalf0 = half0; phalf1 = half1;
        ps0 = start0; ps1 = start1;
        pe0 = end0; pe1 = end1;  // czyli zeby wyliczyc indeks to  x-(c0-1),  x

        if (c0 < (N - 1)/bb) {
            int send_count = half * bb* bb;  // TODO
            cout << "Send" << endl;
            MPI_Isend(sendbuf[odd].data(), send_count, MPI_UNSIGNED_SHORT,
              other, c0+(rank+1)*1000, MPI_COMM_WORLD , &send_req);  // wtedy 2 bufory i wait dla edge
        }
    }

    end_time = omp_get_wtime();

    cout << end_time - start_time << endl;

    // send last blocks (0,N/b i 1,N/b) from rank 1 to rank 0
    unsigned short last_block[2 * bb * bb];

    int ii = 0;
    int jj = last_c1;
    int i0 = bb * ii;
    int j0 = bb * jj;

    if (rank == 1) {
        for (int r = 0; r < 2 * bb; r++) {
            for (int c = 0; c < bb; c++) {
                last_block[r * bb + c] = S[i0 + r][j0 + c];
            }
        }
    }
    MPI_Bcast(last_block, 2 * bb * bb, MPI_UNSIGNED_SHORT, 1, MPI_COMM_WORLD);

    if (rank == 0) {
        for (int r = 0; r < 2 * bb; r++) {
            for (int c = 0; c < bb; c++) {
                S[i0 + r][j0 + c] = last_block[r * bb + c];
            }
        }
    }


    MPI_Finalize();

    if (rank == 0) {
        for (i = N-1; i >= 0; i--) {
            for (j = i+1; j < N; j++) {
                for (k = i; k < j; k++) {
                    S_CPU[i][j] = max(S_CPU[i][k] + S_CPU[k+1][j], S_CPU[i][j]);
                }
                S_CPU[i][j] = max(S_CPU[i][j], S_CPU[i+1][j-1] + paired(seqq[i],seqq[j]));

            }
        }

        for(i=0; i<N; i++)
            for(j=0; j<N; j++)
                if(S[i][j] != S_CPU[i][j]){
                    cout << i <<" " <<  j << ":" << S[i][j] << " " << S_CPU[i][j] << endl;
                    cout << "error" << endl;
                }
    }

    return 0;
}
