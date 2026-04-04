<h1 align="center">
Cache-Efficient and Vectorized Parallel Dynamic Programming for RNA Folding
</h1>

<p align="center">
Code accompanying the research article.
</p>
<hr>

<h2>Overview</h2>

<p>
This repository contains our implementation and experimental study of the <b>Nussinov algorithm</b> together with several related approaches and additional GPU code.
</p>

<ul>
  <li><code>nussinov.cpp</code> – our approach</li>
  <li><code>npdp/</code> – additional methods and baselines</li>
  <li><code>gpu/</code> – CUDA implementation</li>
</ul>

<hr>

<h2>Experimental Data</h2>

<p>
The file <code>experimental_study.xls</code> contains all experimental results used in:
</p>

<ul>
  <li>Tables 1–5</li>
  <li>Figure 2</li>
</ul>

<p>
This file includes the complete dataset required to reproduce the reported results.
</p>

<hr>

<h2>Main implementation</h2>

<p>
The file <code>nussinov.cpp</code> contains <b>our approach</b>.
It can be compiled with vectorization support using:
</p>

<ul>
  <li><code>icpx</code></li>
  <li><code>clang++</code></li>
  <li><code>g++</code></li>
</ul>

<h3>Configuration</h3>

<ul>
  <li>
    <b>Problem size:</b> change <code>N</code> in <code>line 24</code>
  </li>
  <li>
    <b>Number of threads:</b> change <code>num_threads</code> in the
    <code>#pragma omp parallel for</code> directive in <code>line 105</code>
  </li>
</ul>

<hr>

<h2>Additional methods</h2>

<p>
The <code>npdp/</code> subdirectory contains the following methods:
</p>

<ul>
  <li><code>transpose</code></li>
  <li><code>pluto</code></li>
  <li><code>traco</code></li>
  <li><code>dapt</code></li>
</ul>

<p>
See <code>info.txt</code> for additional details.
</p>

<hr>

<h2>GPU version</h2>

<p>
The <code>gpu/</code> subdirectory contains the <b>CUDA version</b>.
</p>

<hr>

<h2>PS4R and PST4R</h2>

<p>
For the PS4R and PST4R implementations, please contact the author:
</p>

<p>
  <b>Ingrid Kamga</b><br>
  <a href="mailto:xingridkamga@gmail.com">xingridkamga@gmail.com</a>
</p>

<p>
Related files are available in:
</p>

<ul>
  <li><code>s4r-nussinov-pack.zip</code></li>
</ul>

<hr>

<h2>Notes</h2>

<p>
This repository is intended to support implementation details, compilation, and comparison of multiple Nussinov-based approaches across CPU and GPU platforms.
</p>
