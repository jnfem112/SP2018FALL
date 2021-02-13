
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>UNIX domain socket tutorial</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//mp2/unix_socket.html">
</head>


  <body>

<!--
    <header class="site-header">
-->

  <div class="wrapper">

<!--
    <a class="site-title" href="/">System Programming Design (Fall 2018) at NTU</a>
-->

  </div>

</header>

<!--
    <div class="page-content">
      <div class="wrapper">
        <article class="post">
-->

  <div class="post-content">
    <h1 id="unix-domain-socket-tutorial">UNIX domain socket tutorial</h1>

<h2 id="brief-into">Brief into</h2>

<p>UNIX domain socket is a <em>inter-process communication</em> mechanism, enabling processes exchange their data. This page aims for usage of UNIX socket.</p>

<p>Suppose process <em>Resol</em> waits for process <em>Reep</em> to send data. To initiate a communication, <em>Resol</em> has to setup a <strong>listening socket</strong> and waits for <em>Reep</em>. <em>Reep</em> creates a <strong>connecting socket</strong>, then connects <em>Resol’s</em> socket file. Once <em>Resol</em> accepts the connection, they can start communications.</p>

<h3 id="setup-a-listening-socket">Setup a listening socket</h3>

<p>On <em>Resol’s</em> side, the procedure goes as follows:</p>

<ol>
  <li><em>Resol</em> creates a socket by calling <code class="highlighter-rouge">socket()</code>.
    <div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="kt">int</span> <span class="n">listen_sock</span> <span class="o">=</span> <span class="n">socket</span><span class="p">(</span><span class="n">AF_UNIX</span><span class="p">,</span> <span class="n">SOCK_STREAM</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span>
</code></pre></div>    </div>
  </li>
  <li><em>Resol</em> assigns the socket a name. In case of UNIX socket, the name is exactly a filename. <em>Resol</em> <code class="highlighter-rouge">bind()</code> the socket with a filename.
    <div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="c1">// We need extra steps to create the _addr_ variable. Here we omit it for simplicity.
</span><span class="n">ret</span> <span class="o">=</span> <span class="n">bind</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="p">(</span><span class="k">const</span> <span class="k">struct</span> <span class="n">sockaddr</span> <span class="o">*</span><span class="p">)</span> <span class="o">&amp;</span><span class="n">addr</span><span class="p">,</span> <span class="k">sizeof</span><span class="p">(</span><span class="k">struct</span> <span class="n">sockaddr_un</span><span class="p">));</span>
</code></pre></div>    </div>
  </li>
  <li><em>Resol</em> make sures the socket is listening type. After this step, you can see a socket file in the filesystem.
    <div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="c1">// The 20 here is called _backlog_
</span><span class="n">ret</span> <span class="o">=</span> <span class="n">listen</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="mi">20</span><span class="p">);</span>
</code></pre></div>    </div>
  </li>
  <li><em>Resol</em> calls <code class="highlighter-rouge">accept()</code> and start to wait. It blocks until <em>Reep’s</em> connection comes.
    <div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="n">data_socket</span> <span class="o">=</span> <span class="n">accept</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="nb">NULL</span><span class="p">,</span> <span class="nb">NULL</span><span class="p">);</span>
</code></pre></div>    </div>
  </li>
  <li>Once <code class="highlighter-rouge">accept()</code> returns, it returns a file descriptor <code class="highlighter-rouge">data_socket</code> here. We can send and receive bytes using <code class="highlighter-rouge">send()</code> and <code class="highlighter-rouge">recv()</code>.
    <div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="n">recv</span><span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="n">buf</span><span class="p">,</span> <span class="n">buf_size</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span>
<span class="n">send</span><span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="n">buf</span><span class="p">,</span> <span class="n">buf_size</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span>
</code></pre></div>    </div>
  </li>
  <li>Like other file descriptors, you have to <code class="highlighter-rouge">close()</code> it once it’s not refered anymore.</li>
</ol>

<p>Note that <em>Resol</em> can <code class="highlighter-rouge">accept()</code> multiple times, such that it can communicate with more than one processes.</p>

<h3 id="setup-a-connecting-socket">Setup a connecting socket</h3>

<p>On <em>Reep’s</em> side, the procedure goes as follows:</p>

<ol>
  <li>
    <p><em>Reep</em> creates a UNIX socket by calling <code class="highlighter-rouge">socket()</code>.</p>
    <div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="n">data_socket</span> <span class="o">=</span> <span class="n">socket</span><span class="p">(</span><span class="n">AF_UNIX</span><span class="p">,</span> <span class="n">SOCK_STREAM</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span>
</code></pre></div>    </div>
  </li>
  <li>It calls <code class="highlighter-rouge">connect()</code> to connect to <em>Resol’s</em> socket file. Of course, the address should be identical to the <em>Resol’s</em> socket filename.
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>// We need extra steps to setup _addr_ beforehand.
ret = connect(data_socket, (const struct sockaddr *) &amp;addr, sizeof(struct sockaddr_un));
</code></pre></div>    </div>
  </li>
  <li>If <code class="highlighter-rouge">connect()</code> returns without errors, <em>Reep</em> now can start to <code class="highlighter-rouge">send()</code> and <code class="highlighter-rouge">recv()</code> data.
    <div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="n">recv</span><span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="n">buf</span><span class="p">,</span> <span class="n">buf_size</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span>
<span class="n">send</span><span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="n">buf</span><span class="p">,</span> <span class="n">buf_size</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span>
</code></pre></div>    </div>
  </li>
  <li>Of course, you have to <code class="highlighter-rouge">close()</code> it eventually.</li>
</ol>

<h2 id="update-3-complete-example"><strong>(update 3)</strong> Complete example</h2>

<p>The example code is slightly modified from <a href="http://man7.org/linux/man-pages/man7/unix.7.html">unix(7) manpage</a>. The code is only for reference. Please fill missing details in this code.</p>

<h3 id="resols-code-server"><em>Resol’s</em> code (Server)</h3>

<div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="cp">#define SOCKET_NAME "/tmp/resol.sock"
#define BUFFER_SIZE 12
</span>
<span class="cp">#include &lt;stdio.h&gt;
#include &lt;stdlib.h&gt;
#include &lt;string.h&gt;
#include &lt;sys/socket.h&gt;
#include &lt;sys/un.h&gt;
#include &lt;unistd.h&gt;
</span>
<span class="kt">int</span>
<span class="nf">main</span><span class="p">(</span><span class="kt">int</span> <span class="n">argc</span><span class="p">,</span> <span class="kt">char</span> <span class="o">*</span><span class="n">argv</span><span class="p">[])</span>
<span class="p">{</span>
    <span class="k">struct</span> <span class="n">sockaddr_un</span> <span class="n">addr</span><span class="p">;</span>
    <span class="kt">int</span> <span class="n">down_flag</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span>
    <span class="kt">int</span> <span class="n">ret</span><span class="p">;</span>
    <span class="kt">int</span> <span class="n">listen_socket</span><span class="p">;</span>
    <span class="kt">int</span> <span class="n">data_socket</span><span class="p">;</span>
    <span class="kt">int</span> <span class="n">result</span><span class="p">;</span>
    <span class="kt">char</span> <span class="n">buffer</span><span class="p">[</span><span class="n">BUFFER_SIZE</span><span class="p">];</span>
<br>
    <span class="cm">/*
     * In case the program exited inadvertently on the last run,
     * remove the socket.
     */</span>
<br>
    <span class="n">unlink</span><span class="p">(</span><span class="n">SOCKET_NAME</span><span class="p">);</span>
<br>
    <span class="cm">/* Create local socket. */</span>
<br>
    <span class="n">listen_socket</span> <span class="o">=</span> <span class="n">socket</span><span class="p">(</span><span class="n">AF_UNIX</span><span class="p">,</span> <span class="n">SOCK_STREAM</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span>
    <span class="k">if</span> <span class="p">(</span><span class="n">listen_socket</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
        <span class="n">perror</span><span class="p">(</span><span class="s">"socket"</span><span class="p">);</span>
        <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
    <span class="p">}</span>
<br>
    <span class="cm">/*
     * For portability clear the whole structure, since some
     * implementations have additional (nonstandard) fields in
     * the structure.
     */</span>
<br>
    <span class="n">memset</span><span class="p">(</span><span class="o">&amp;</span><span class="n">addr</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="k">sizeof</span><span class="p">(</span><span class="k">struct</span> <span class="n">sockaddr_un</span><span class="p">));</span>
<br>
    <span class="cm">/* Bind socket to socket name. */</span>
<br>
    <span class="n">addr</span><span class="p">.</span><span class="n">sun_family</span> <span class="o">=</span> <span class="n">AF_UNIX</span><span class="p">;</span>
    <span class="n">strncpy</span><span class="p">(</span><span class="n">addr</span><span class="p">.</span><span class="n">sun_path</span><span class="p">,</span> <span class="n">SOCKET_NAME</span><span class="p">,</span> <span class="k">sizeof</span><span class="p">(</span><span class="n">addr</span><span class="p">.</span><span class="n">sun_path</span><span class="p">)</span> <span class="o">-</span> <span class="mi">1</span><span class="p">);</span>
<br>
    <span class="n">ret</span> <span class="o">=</span> <span class="n">bind</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="p">(</span><span class="k">const</span> <span class="k">struct</span> <span class="n">sockaddr</span> <span class="o">*</span><span class="p">)</span> <span class="o">&amp;</span><span class="n">addr</span><span class="p">,</span>
               <span class="k">sizeof</span><span class="p">(</span><span class="k">struct</span> <span class="n">sockaddr_un</span><span class="p">));</span>
    <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
        <span class="n">perror</span><span class="p">(</span><span class="s">"bind"</span><span class="p">);</span>
        <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
    <span class="p">}</span>
<br>
    <span class="cm">/*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     */</span>
<br>
    <span class="n">ret</span> <span class="o">=</span> <span class="n">listen</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="mi">20</span><span class="p">);</span>
    <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
        <span class="n">perror</span><span class="p">(</span><span class="s">"listen"</span><span class="p">);</span>
        <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
    <span class="p">}</span>
<br>
    <span class="cm">/* This is the main loop for handling connections. */</span>
<br>
    <span class="k">for</span> <span class="p">(;;)</span> <span class="p">{</span>
<br>
        <span class="cm">/* Wait for incoming connection. */</span>
<br>
        <span class="n">data_socket</span> <span class="o">=</span> <span class="n">accept</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="nb">NULL</span><span class="p">,</span> <span class="nb">NULL</span><span class="p">);</span>
        <span class="k">if</span> <span class="p">(</span><span class="n">data_socket</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
            <span class="n">perror</span><span class="p">(</span><span class="s">"accept"</span><span class="p">);</span>
            <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
        <span class="p">}</span>
<br>
        <span class="n">result</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span>
        <span class="k">for</span><span class="p">(;;)</span> <span class="p">{</span>
<br>
            <span class="cm">/* Wait for next data packet. */</span>
<br>
            <span class="n">ret</span> <span class="o">=</span> <span class="n">read</span><span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="n">buffer</span><span class="p">,</span> <span class="n">BUFFER_SIZE</span><span class="p">);</span>
            <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
                <span class="n">perror</span><span class="p">(</span><span class="s">"read"</span><span class="p">);</span>
                <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
            <span class="p">}</span>
<br>
            <span class="cm">/* Ensure buffer is 0-terminated. */</span>
<br>
            <span class="n">buffer</span><span class="p">[</span><span class="n">BUFFER_SIZE</span> <span class="o">-</span> <span class="mi">1</span><span class="p">]</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span>
<br>
            <span class="cm">/* Handle commands. */</span>
<br>
            <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="n">strncmp</span><span class="p">(</span><span class="n">buffer</span><span class="p">,</span> <span class="s">"DOWN"</span><span class="p">,</span> <span class="n">BUFFER_SIZE</span><span class="p">))</span> <span class="p">{</span>
                <span class="n">down_flag</span> <span class="o">=</span> <span class="mi">1</span><span class="p">;</span>
                <span class="k">break</span><span class="p">;</span>
            <span class="p">}</span>
<br>
            <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="n">strncmp</span><span class="p">(</span><span class="n">buffer</span><span class="p">,</span> <span class="s">"END"</span><span class="p">,</span> <span class="n">BUFFER_SIZE</span><span class="p">))</span> <span class="p">{</span>
                <span class="k">break</span><span class="p">;</span>
            <span class="p">}</span>
<br>
            <span class="cm">/* Add received summand. */</span>
<br>
            <span class="n">result</span> <span class="o">+=</span> <span class="n">atoi</span><span class="p">(</span><span class="n">buffer</span><span class="p">);</span>
        <span class="p">}</span>
<br>
        <span class="cm">/* Send result. */</span>
<br>
        <span class="n">sprintf</span><span class="p">(</span><span class="n">buffer</span><span class="p">,</span> <span class="s">"%d"</span><span class="p">,</span> <span class="n">result</span><span class="p">);</span>
        <span class="n">ret</span> <span class="o">=</span> <span class="n">write</span><span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="n">buffer</span><span class="p">,</span> <span class="n">BUFFER_SIZE</span><span class="p">);</span>
<br>
        <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
            <span class="n">perror</span><span class="p">(</span><span class="s">"write"</span><span class="p">);</span>
            <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
        <span class="p">}</span>
<br>
        <span class="cm">/* Close socket. */</span>
<br>
        <span class="n">close</span><span class="p">(</span><span class="n">data_socket</span><span class="p">);</span>
<br>
        <span class="cm">/* Quit on DOWN command. */</span>
<br>
        <span class="k">if</span> <span class="p">(</span><span class="n">down_flag</span><span class="p">)</span> <span class="p">{</span>
            <span class="k">break</span><span class="p">;</span>
        <span class="p">}</span>
    <span class="p">}</span>
<br>
    <span class="n">close</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">);</span>
<br>
    <span class="cm">/* Unlink the socket. */</span>
<br>
    <span class="n">unlink</span><span class="p">(</span><span class="n">SOCKET_NAME</span><span class="p">);</span>
<br>
    <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_SUCCESS</span><span class="p">);</span>
<span class="p">}</span>
</code></pre></div></div>

<h3 id="reeps-code-client"><em>Reep’s</em> code (Client)</h3>

<div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="cp">#define SOCKET_NAME "/tmp/resol.sock"
#define BUFFER_SIZE 12
</span>
<span class="cp">#include &lt;errno.h&gt;
#include &lt;stdio.h&gt;
#include &lt;stdlib.h&gt;
#include &lt;string.h&gt;
#include &lt;sys/socket.h&gt;
#include &lt;sys/un.h&gt;
#include &lt;unistd.h&gt;
</span>
<span class="kt">int</span>
<span class="nf">main</span><span class="p">(</span><span class="kt">int</span> <span class="n">argc</span><span class="p">,</span> <span class="kt">char</span> <span class="o">*</span><span class="n">argv</span><span class="p">[])</span>
<span class="p">{</span>
    <span class="k">struct</span> <span class="n">sockaddr_un</span> <span class="n">addr</span><span class="p">;</span>
    <span class="kt">int</span> <span class="n">i</span><span class="p">;</span>
    <span class="kt">int</span> <span class="n">ret</span><span class="p">;</span>
    <span class="kt">int</span> <span class="n">data_socket</span><span class="p">;</span>
    <span class="kt">char</span> <span class="n">buffer</span><span class="p">[</span><span class="n">BUFFER_SIZE</span><span class="p">];</span>
<br>
    <span class="cm">/* Create local socket. */</span>
<br>
    <span class="n">data_socket</span> <span class="o">=</span> <span class="n">socket</span><span class="p">(</span><span class="n">AF_UNIX</span><span class="p">,</span> <span class="n">SOCK_STREAM</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span>
    <span class="k">if</span> <span class="p">(</span><span class="n">data_socket</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
        <span class="n">perror</span><span class="p">(</span><span class="s">"socket"</span><span class="p">);</span>
        <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
    <span class="p">}</span>
<br>
    <span class="cm">/*
     * For portability clear the whole structure, since some
     * implementations have additional (nonstandard) fields in
     * the structure.
     */</span>
<br>
    <span class="n">memset</span><span class="p">(</span><span class="o">&amp;</span><span class="n">addr</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="k">sizeof</span><span class="p">(</span><span class="k">struct</span> <span class="n">sockaddr_un</span><span class="p">));</span>
<br>
    <span class="cm">/* Connect socket to socket address */</span>
<br>
    <span class="n">addr</span><span class="p">.</span><span class="n">sun_family</span> <span class="o">=</span> <span class="n">AF_UNIX</span><span class="p">;</span>
    <span class="n">strncpy</span><span class="p">(</span><span class="n">addr</span><span class="p">.</span><span class="n">sun_path</span><span class="p">,</span> <span class="n">SOCKET_NAME</span><span class="p">,</span> <span class="k">sizeof</span><span class="p">(</span><span class="n">addr</span><span class="p">.</span><span class="n">sun_path</span><span class="p">)</span> <span class="o">-</span> <span class="mi">1</span><span class="p">);</span>
<br>
    <span class="n">ret</span> <span class="o">=</span> <span class="n">connect</span> <span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="p">(</span><span class="k">const</span> <span class="k">struct</span> <span class="n">sockaddr</span> <span class="o">*</span><span class="p">)</span> <span class="o">&amp;</span><span class="n">addr</span><span class="p">,</span>
                   <span class="k">sizeof</span><span class="p">(</span><span class="k">struct</span> <span class="n">sockaddr_un</span><span class="p">));</span>
    <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
        <span class="n">fprintf</span><span class="p">(</span><span class="n">stderr</span><span class="p">,</span> <span class="s">"The server is down.</span><span class="se">\n</span><span class="s">"</span><span class="p">);</span>
        <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
    <span class="p">}</span>
<br>
    <span class="cm">/* Send arguments. */</span>
<br>
    <span class="k">for</span> <span class="p">(</span><span class="n">i</span> <span class="o">=</span> <span class="mi">1</span><span class="p">;</span> <span class="n">i</span> <span class="o">&lt;</span> <span class="n">argc</span><span class="p">;</span> <span class="o">++</span><span class="n">i</span><span class="p">)</span> <span class="p">{</span>
        <span class="n">ret</span> <span class="o">=</span> <span class="n">write</span><span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="n">argv</span><span class="p">[</span><span class="n">i</span><span class="p">],</span> <span class="n">strlen</span><span class="p">(</span><span class="n">argv</span><span class="p">[</span><span class="n">i</span><span class="p">])</span> <span class="o">+</span> <span class="mi">1</span><span class="p">);</span>
        <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
            <span class="n">perror</span><span class="p">(</span><span class="s">"write"</span><span class="p">);</span>
            <span class="k">break</span><span class="p">;</span>
        <span class="p">}</span>
    <span class="p">}</span>
<br>
    <span class="cm">/* Request result. */</span>
<br>
    <span class="n">strcpy</span> <span class="p">(</span><span class="n">buffer</span><span class="p">,</span> <span class="s">"END"</span><span class="p">);</span>
    <span class="n">ret</span> <span class="o">=</span> <span class="n">write</span><span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="n">buffer</span><span class="p">,</span> <span class="n">strlen</span><span class="p">(</span><span class="n">buffer</span><span class="p">)</span> <span class="o">+</span> <span class="mi">1</span><span class="p">);</span>
    <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
        <span class="n">perror</span><span class="p">(</span><span class="s">"write"</span><span class="p">);</span>
        <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
    <span class="p">}</span>
<br>
    <span class="cm">/* Receive result. */</span>
<br>
    <span class="n">ret</span> <span class="o">=</span> <span class="n">read</span><span class="p">(</span><span class="n">data_socket</span><span class="p">,</span> <span class="n">buffer</span><span class="p">,</span> <span class="n">BUFFER_SIZE</span><span class="p">);</span>
    <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
        <span class="n">perror</span><span class="p">(</span><span class="s">"read"</span><span class="p">);</span>
        <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
    <span class="p">}</span>
<br>
    <span class="cm">/* Ensure buffer is 0-terminated. */</span>
<br>
    <span class="n">buffer</span><span class="p">[</span><span class="n">BUFFER_SIZE</span> <span class="o">-</span> <span class="mi">1</span><span class="p">]</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span>
<br>
    <span class="n">printf</span><span class="p">(</span><span class="s">"Result = %s</span><span class="se">\n</span><span class="s">"</span><span class="p">,</span> <span class="n">buffer</span><span class="p">);</span>
<br>
    <span class="cm">/* Close socket. */</span>
<br>
    <span class="n">close</span><span class="p">(</span><span class="n">data_socket</span><span class="p">);</span>
<br>
    <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_SUCCESS</span><span class="p">);</span>
<span class="p">}</span>
</code></pre></div></div>

<h2 id="update-4-communicate-with-multiple-clients"><strong>(Update 4)</strong> Communicate with multiple clients</h2>

<p>Since <em>Resol</em> can <code class="highlighter-rouge">accept()</code> more than once, it’s possible that <em>Resol</em> can exchange data with more than one processes.</p>

<p>We are aware that <code class="highlighter-rouge">send()</code> and <code class="highlighter-rouge">recv()</code> are blocking calls. For example, if <em>Resol</em> calls <code class="highlighter-rouge">send()</code> but <em>Reep</em> refuses to <code class="highlighter-rouge">recv()</code>, <em>Resol</em> will stuck at that place as well as other clients.</p>

<p>To cope with this issue, <code class="highlighter-rouge">select()</code> would be our friend. We can form a set of file descriptors and hand it to <code class="highlighter-rouge">select()</code>. Once one of the fds is ready to read or write, <code class="highlighter-rouge">select()</code> returns and we can find which fd is available.</p>

<p>The code is only for reference. Please fill missing details in this code.</p>

<div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="cp">#include &lt;stdio.h&gt;
#include &lt;stdlib.h&gt;
#include &lt;string.h&gt;
#include &lt;sys/time.h&gt;
#include &lt;sys/types.h&gt;
#include &lt;sys/socket.h&gt;
#include &lt;sys/un.h&gt;
#include &lt;unistd.h&gt;
</span>
<span class="kt">int</span>
<span class="nf">main</span><span class="p">(</span><span class="kt">void</span><span class="p">)</span>
<span class="p">{</span>
    <span class="kt">int</span> <span class="n">ret</span><span class="p">;</span>
    <span class="kt">int</span> <span class="n">listen_socket</span><span class="p">;</span>
<br>
    <span class="cm">/* Setup a listen socket, bind() and listen() on it */</span>
    <span class="n">listen_socket</span> <span class="o">=</span> <span class="n">socket</span><span class="p">(</span><span class="n">AF_UNIX</span><span class="p">,</span> <span class="n">SOCK_STREAM</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span>
    <span class="c1">// ...skip
</span>    <span class="n">ret</span> <span class="o">=</span> <span class="n">bind</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="p">(</span><span class="k">const</span> <span class="k">struct</span> <span class="n">sockaddr</span> <span class="o">*</span><span class="p">)</span> <span class="o">&amp;</span><span class="n">addr</span><span class="p">,</span> <span class="k">sizeof</span><span class="p">(</span><span class="k">struct</span> <span class="n">sockaddr_un</span><span class="p">));</span>
    <span class="c1">// ...skip
</span>    <span class="n">ret</span> <span class="o">=</span> <span class="n">listen</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="mi">20</span><span class="p">);</span>
<br>
    <span class="n">fd_set</span> <span class="n">rfds</span><span class="p">;</span>
<br>
    <span class="cm">/* Watch stdin and listen socket to see when it has input */</span>
<br>
    <span class="n">FD_ZERO</span><span class="p">(</span><span class="o">&amp;</span><span class="n">rfds</span><span class="p">);</span>
    <span class="n">FD_SET</span><span class="p">(</span><span class="mi">0</span><span class="p">,</span> <span class="o">&amp;</span><span class="n">rfds</span><span class="p">);</span>
    <span class="n">FD_SET</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="o">&amp;</span><span class="n">rfds</span><span class="p">);</span>
<br>
    <span class="cm">/* Wait up to five seconds. */</span>
<br>
    <span class="k">for</span> <span class="p">(;;)</span>
    <span class="p">{</span>
        <span class="cm">/* We make a copy of original set because select() will mess it up */</span>
        <span class="n">fd_set</span> <span class="n">working_rfds</span><span class="p">;</span>
        <span class="n">memcpy</span><span class="p">(</span><span class="o">&amp;</span><span class="n">working_rfds</span><span class="p">,</span> <span class="o">&amp;</span><span class="n">rfds</span><span class="p">,</span> <span class="k">sizeof</span><span class="p">(</span><span class="n">working_rfds</span><span class="p">));</span>
<br>
        <span class="n">ret</span> <span class="o">=</span> <span class="n">select</span><span class="p">(</span><span class="mi">1024</span><span class="p">,</span> <span class="o">&amp;</span><span class="n">working_rfds</span><span class="p">,</span> <span class="nb">NULL</span><span class="p">,</span> <span class="nb">NULL</span><span class="p">,</span> <span class="nb">NULL</span><span class="p">);</span>
<br>
        <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span>
            <span class="n">perror</span><span class="p">(</span><span class="s">"select()"</span><span class="p">);</span>
            <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_FAILURE</span><span class="p">);</span>
        <span class="p">}</span>
<br>
        <span class="k">if</span> <span class="p">(</span><span class="n">ret</span> <span class="o">==</span> <span class="mi">0</span><span class="p">)</span> <span class="p">{</span>
            <span class="n">printf</span><span class="p">(</span><span class="s">"No data is available, we skip this loop</span><span class="se">\n</span><span class="s">"</span><span class="p">);</span>
            <span class="k">continue</span><span class="p">;</span>
        <span class="p">}</span>
<br>
        <span class="cm">/* After this line, input data is available */</span>
<br>
        <span class="k">if</span> <span class="p">(</span><span class="n">FD_ISSET</span><span class="p">(</span><span class="mi">0</span><span class="p">,</span> <span class="o">&amp;</span><span class="n">working_rfds</span><span class="p">))</span> <span class="p">{</span>
            <span class="cm">/* read stdin */</span>
        <span class="p">}</span>
<br>
        <span class="k">if</span> <span class="p">(</span><span class="n">FD_ISSET</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="o">&amp;</span><span class="n">working_rfds</span><span class="p">))</span> <span class="p">{</span>
            <span class="kt">int</span> <span class="n">data_socket</span> <span class="o">=</span> <span class="n">accept</span><span class="p">(</span><span class="n">listen_socket</span><span class="p">,</span> <span class="nb">NULL</span><span class="p">,</span> <span class="nb">NULL</span><span class="p">);</span>
            <span class="cm">/* communicate with client */</span>
        <span class="p">}</span>
<br>
    <span class="p">}</span>
<br>
    <span class="n">exit</span><span class="p">(</span><span class="n">EXIT_SUCCESS</span><span class="p">);</span>
<span class="p">}</span>
</code></pre></div></div>

  </div>

</article>

<!--
      </div>
    </div>
-->

  </body>

</html>
