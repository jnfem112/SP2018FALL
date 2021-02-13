
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>MP0 Q&amp;amp;A</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//mp0/QandA.html">
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
    <h2 id="mp0-qa">MP0 Q&amp;A</h2>

<h3 id="how-to-write-to-standard-error">How to write to <em>standard error</em>?</h3>
<p>This is the easiest one. Of course, there are alternatives.</p>
<div class="language-c highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="n">fprintf</span><span class="p">(</span><span class="n">stderr</span><span class="p">,</span> <span class="s">"error</span><span class="se">\n</span><span class="s">"</span><span class="p">);</span>
</code></pre></div></div>

<h3 id="good-compilation-practices">Good compilation practices</h3>
<p>With <code class="highlighter-rouge">-Wall</code> option, GCC compiler produces warnings about constructions that some users consider questionable. This helps avoiding bugs in your code.</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>gcc <span class="nt">-o</span> char_count char_count.c <span class="nt">-Wall</span>
</code></pre></div></div>

<h3 id="how-to-terminate-your-program-if-it-hangs">How to terminate your program if it hangs</h3>
<p>Press ^C (<em>Ctrl-C</em>) or ^\ (<em>Ctrl-\</em>). The process will receive SIGINT or SIGQUIT signal. If neither of these works, start another terminal and run <code class="highlighter-rouge">killall -KILL char_count</code>.</p>

<h3 id="how-to-generate-input-for-testing">How to generate input for testing</h3>
<p><code class="highlighter-rouge">yes</code>, <code class="highlighter-rouge">head</code>, <code class="highlighter-rouge">tail</code>, <code class="highlighter-rouge">grep</code>, <code class="highlighter-rouge">shuf</code> commands are friends. See manpages for detailed usage. For example, <code class="highlighter-rouge">man yes</code> shows to manual for <code class="highlighter-rouge">yes</code> command.</p>

<p>To test a file with 1000 line of text,</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>yes | head <span class="nt">-n</span> 1000 <span class="o">&gt;</span> FILE.txt
./char_count <span class="s2">"ab c"</span> FILE.txt
</code></pre></div></div>

<p>To feed your program with infinitely many lines,</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>yes | ./char_count
</code></pre></div></div>

  </div>

</article>

<!--
      </div>
    </div>
-->

  </body>

</html>
