
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>Example Usages</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//mp0/EXAMPLE.html">
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
    <h2 id="example-usages">Example Usages</h2>

<h3 id="read-from-a-file">Read from a file</h3>

<p>Suppose a file <em>input.txt</em> with the following content. ‘\n’ indicates the line break character.</p>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>Ada\n
Grace Hopper\n
Margaret Hamilton\n
</code></pre></div></div>

<p>We count the occurences in any of “abc”.</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>./char_count abc input.txt
</code></pre></div></div>

<p>Your program should produces the following output.</p>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>1\n
2\n
3\n
</code></pre></div></div>

<p>Once the INPUT_FILE does not exist or is a directory, our program prints “error\n” to <em>standard error</em>.</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>./char_count abc a_nonexisting_file.txt
</code></pre></div></div>

<p>the <em>standard error</em>:</p>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>error\n
</code></pre></div></div>

<h3 id="read-from-standard-input">Read from <em>standard input</em></h3>

<p>You can test this feature by redirection techniques. Note that <em>input.txt</em> is no longer an command argument. It’s comsumed by &lt; redirection symbol.</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>./char_count abc &lt; input.txt
</code></pre></div></div>

<p>or you can type the input from terminal directly, and end the input by sending ^D (<em>Ctrl+D</em>).</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>./char_count abc
Ada<span class="se">\n</span>
Grace Hopper<span class="se">\n</span>
Margaret Hamilton<span class="se">\n</span>
^D
</code></pre></div></div>

<p>Another scenario making <em>standard input</em> useful is that your program is <em>pipelined</em>. This technique makes it possible that your program consumes from other program’s output. TAs use similar tricks to judge large input case.</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>./TAs_large_text_generator | ./char_count abc
</code></pre></div></div>

<h3 id="specify-an-empty-string-as-charset">Specify an empty string as CHARSET</h3>

<p>You can use single or double quotes to specify an empty argument. Be aware that single quotes and double quotes won’t be parsed in the same way (see this <a href="https://stackoverflow.com/questions/6697753/difference-between-single-and-double-quotes-in-bash">StackOverflow</a>). However, in this case the effect is the same.</p>

<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="c"># use double quotes</span>
./char_count <span class="s2">""</span> input.txt
<br>
<span class="c"># or single quotes</span>
./char_count <span class="s1">''</span> input.txt
<br>
<span class="c"># this one is wrong since input.txt is treated as this first argument</span>
<span class="c"># ./char_count input.txt</span>
</code></pre></div></div>

  </div>

</article>

<!--
      </div>
    </div>
-->

  </body>

</html>
