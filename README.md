<h1 align="center">Nussinov Implementations</h1>

<p align="center">
  Reference implementations and comparison baselines for the Nussinov algorithm.
</p>

<hr>

<h2>Overview</h2>

<p>
This repository contains our implementation of the <b>Nussinov algorithm</b> together with several related approaches and additional GPU code.
</p>

<ul>
  <li><code>nussinov.cpp</code> – our approach</li>
  <li><code>npdp/</code> – additional methods and baselines</li>
  <li><code>gpu/</code> – CUDA implementation</li>
</ul>

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
