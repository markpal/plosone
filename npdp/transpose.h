void li()
{
    int i,j,k;
    int diag,row,col;
    char a,b;
    int _max,t;


    #pragma omp parallel for
    for(i=0; i<=N-1; i++)
     S[i][i] = 0;

    #pragma omp parallel for
    for(i=0; i<=N-2; i++)
     S[i][i+1] = 0;

    for(diag=1; diag<=N-1; diag++){
    #pragma omp parallel for private(row, col, _max, t, k) shared(diag, RNA) 
      for(row=0; row<=N-diag-1; row++){
        col = diag + row;
        a = RNA[row];
        b = RNA[col];
        _max = S[row+1][col-1] + can_pair(RNA, row, col);
        for(k=row; k <=col-1; k++){
         t = S[row][k] + S[col][k+1];
         _max = max(_max, t);
       }
       S[row][col] = S[col][row] =  _max;
       }
    }



}
