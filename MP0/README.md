
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>SP18MP0</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//MP0/">
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

<h1 id="machine-problem-0---introduction-to-system-programming">Machine Problem 0 - Introduction to Systems Programming</h1>

<ul>
  <li>Due Date: 11:59 PM, Oct. 2, 2018</li>
  <li>Late submission: We don’t accept late submission in this homework.</li>
</ul>

<p>In this homework, you will practice Linux programming and uploading your homework to GitHub.</p>

<hr />

  <div class="post-content">
    <ul id="markdown-toc">
  <li><a href="#prerequisite" id="markdown-toc-prerequisite">Prerequisite</a>    <ul>
      <li><a href="#join-github" id="markdown-toc-join-github">Join GitHub</a></li>
      <li><a href="#programming-on-the-workstation" id="markdown-toc-programming-on-the-workstation">Programming on the Workstation</a></li>
    </ul>
  </li>
  <li><a href="#description" id="markdown-toc-description">Description</a>    <ul>
      <li><a href="#work-on-your-repository" id="markdown-toc-work-on-your-repository">Work on your repository</a></li>
      <li><a href="#file-hierarchy" id="markdown-toc-file-hierarchy">File Hierarchy</a></li>
      <li><a href="#makefile" id="markdown-toc-makefile">Makefile</a></li>
      <li><a href="#program-usage" id="markdown-toc-program-usage">Program Usage</a>        <ul>
          <li><a href="#time-limit" id="markdown-toc-time-limit">Time Limit</a></li>
          <li><a href="#notes" id="markdown-toc-notes">Notes</a></li>
        </ul>
      </li>
      <li><a href="#example" id="markdown-toc-example">Example</a></li>
    </ul>
  </li>
  <li><a href="#submit-your-homework-to-github" id="markdown-toc-submit-your-homework-to-github">Submit your homework to GitHub</a></li>
  <li><a href="#gradings" id="markdown-toc-gradings">Gradings</a></li>
  <li><a href="#supplementary-materials" id="markdown-toc-supplementary-materials">Supplementary Materials</a></li>
</ul>

<h2 id="prerequisite">Prerequisite</h2>

<h3 id="join-github">Join GitHub</h3>
<ol>
  <li>Create an account on <a href="https://github.com/">GitHub</a>.</li>
  <li>Fill <a href="https://docs.google.com/forms/d/e/1FAIpQLSe2yh6ntEple-hrdrFWbSWxpPYljX9bog7ohXjaZiWor4irdA/viewform">this form</a> if not yet do it.</li>
</ol>

<h3 id="programming-on-the-workstation">Programming on the Workstation</h3>
<p>If you don’t have a CSIE workstation account, refer to <a href="https://wslab.csie.ntu.edu.tw/account.html">this page</a> to apply an account.</p>

<h2 id="description">Description</h2>

<p>In this homework, you are required to write a simple C/C++ program and a Makefile, and push your code to your repository on GitHub. TA will only receive your code after you push them to the remote repository.</p>

<h3 id="work-on-your-repository">Work on your repository</h3>

<p>Your GitHub repository page is located at <code class="highlighter-rouge">https://github.com/SystemProgrammingatNTU/SP18-&lt;StudentID&gt;/</code>. Notify TA if anything goes wrong.</p>

<p>If it’s the first time you download the repo, run <code class="highlighter-rouge">git clone</code> and your repo <code class="highlighter-rouge">SP18-&lt;StudentID&gt;</code> will be created.</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>git clone https://github.com/SystemProgrammingatNTU/SP18-&lt;StudentID&gt;.git
<span class="nb">cd </span>SP18-&lt;StudentID&gt;
</code></pre></div></div>

<p>and repeat the <em>modify-add-commit</em> cycle to update your code until your job is done.</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code><span class="o">(</span>create files, delete stuffs, update your code, etc<span class="o">)</span>
...
git add file.a some.b bla.c
git commit <span class="nt">-m</span> <span class="s1">'The job description'</span>
</code></pre></div></div>

<p>Run <code class="highlighter-rouge">git push</code> to upload your code to the remote repository.</p>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code>git push
</code></pre></div></div>

<p>You may google <code class="highlighter-rouge">git tutorial</code> or <code class="highlighter-rouge">git 教學</code> keywords to understand Git in depth.</p>

<h3 id="file-hierarchy">File Hierarchy</h3>

<p>Make a directory named <em>MP0</em> in your repository. The <em>MP0</em> dir contains the Makefile and source files.</p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>repo
└── MP0
    ├── Makefile
    └── other files
</code></pre></div></div>

<h3 id="makefile">Makefile</h3>

<p>We’ll compile your code with <code class="highlighter-rouge">make</code> command. Write a Makefile that compiles the source code into an executable file <code class="highlighter-rouge">char_count</code> under the same directory.</p>

<h3 id="program-usage">Program Usage</h3>
<div class="language-sh highlighter-rouge"><div class="highlight"><pre class="highlight"><code> ./char_count CHARSET <span class="o">[</span>INPUT_FILE]
</code></pre></div></div>

<ul>
  <li>
    <p>CHARSET<br>
      A set of <a href="http://www.cplusplus.com/reference/cctype/isprint/">printable characters</a> and <a href="http://www.cplusplus.com/reference/cctype/isspace/">spaces</a>. Characters are guaranteed to be unique.
    </p>
  </li>
  <li>
    <p>INPUT_FILE<br>
      It’s an optional filename. Your program reads from it if specified. Otherwise your program reads from <em>standard input</em>. In the case INPUT_FILE does not exist, your program should print “error\n” to <em>standard error</em> and exit normally.
    </p>
  </li>
</ul>

<p>The input consists of printable characters, spaces, and ‘\n’. Your job is to count the occurences of characters in CHARSET for each line, and print the number of occurences respectively.</p>

<p>For example, suppose the CHARSET is “xyz” and your program encounters a line “<strong>y</strong>ep.tar.<strong>xz</strong>\n”, it should output “3\n” (‘\n’ is new line character). Other examples are listed in <a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP0/README/EXAMPLE.md">example page</a>. Implementation details can be found in <a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP0/README/Q%26A.md">Q&amp;A page</a>.</p>

<h4 id="time-limit">Time Limit</h4>
<ul>
  <li>Small input (&lt;= 100MB) in 10 seconds</li>
  <li>Large input (~ 1GB) in 80 seconds</li>
</ul>

<h4 id="notes">Notes</h4>
<ul>
  <li>Every line is guaranteed to have trailing ‘\n’ in input.</li>
  <li>INPUT_FILE is guaranteed to be a regular file or a non-existing path.</li>
  <li>CHARSET can be empty.</li>
  <li>The matching is case-sensitive.</li>
  <li><strong>The input can be arbitrarily large and lines can be arbitrarily long.</strong></li>
</ul>

<h3 id="example"><a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP0/README/EXAMPLE.md">Example</a></h3>

<h2 id="submit-your-homework-to-github">Submit your homework to GitHub</h2>

<p>TAs regularly check the form and create your repository on GitHub. Your repo page should be on <code class="highlighter-rouge">https://github.com/SystemProgrammingatNTU/SP18-&lt;StudentID&gt;</code>. Notify TAs if anything goes wrong.</p>

<p>Make sure you <code class="highlighter-rouge">git push</code> to upload your code. <strong>The commit with last modification under MP0 directory (on master branch) is judged.</strong> The commits with commit date exceeding the due date are ignored.</p>

<h2 id="gradings">Gradings</h2>
<p>10 points in total. You get the points if anyone of these is completed.</p>
<ul>
  <li>(2 pts) Create Makefile and <code class="highlighter-rouge">make</code> compiles the <code class="highlighter-rouge">char_count</code> binary without error.</li>
  <li>(3 pts) <code class="highlighter-rouge">char_count</code> calculates the counts correctly and ends without error.</li>
  <li>(2 pts) <code class="highlighter-rouge">char_count</code> can print “error\n” to <em>standard error</em> with questionable INPUT_FILE.</li>
  <li>(3 pts) <code class="highlighter-rouge">char_count</code> can handle extra large input (at least 1 GiB).</li>
  <li>(1 pt)  Early bird point. You will get this point if you upload your code before Sep. 25 and don’t get all points.</li>
</ul>

<p>Be aware that <strong>the commit with last modification under MP0 directory (on master branch) is judged.</strong> Plagiarism is prohibited, or you may receive F in this course.</p>

<h2 id="supplementary-materials">Supplementary Materials</h2>

<ul>
  <li>
    <p>Manpages<br>
      <code class="highlighter-rouge">man fprintf</code> shows the manual for <code class="highlighter-rouge">fprintf</code> function in C standard library, <code class="highlighter-rouge">man ls</code> shows the manual for <code class="highlighter-rouge">ls</code> shell command, etc.
    </p>
  </li>
  <li>Git
    <ol>
      <li><a href="http://backlogtool.com/git-guide/en/">Git Beginner’s Guide for Dummies</a></li>
      <li><a href="https://backlogtool.com/git-guide/tw/">連猴子都能懂的Git入門指南</a></li>
    </ol>
  </li>
  <li>Vim
    <ol>
      <li><a href="http://linux.vbird.org/linux_basic/0310vi.php">鳥哥的 Linux 私房菜 - 第九章、vim 程式編輯器</a></li>
      <li><a href="http://vim-adventures.com/">Vim Adventure</a></li>
      <li><a href="http://vimawesome.com/">Vim Awesome</a></li>
      <li><a href="http://www.vixual.net/blog/archives/234">vi 與 vim 的指令整理</a></li>
    </ol>
  </li>
  <li>Makefile
    <ol>
      <li><a href="http://mropengate.blogspot.com/2018/01/makefile.html">簡單學 makefile - makefile 教學 (1)</a></li>
      <li><a href="https://www.tutorialspoint.com/makefile/">Unix Makefile Tutorial</a></li>
      <li><a href="https://spin.atomicobject.com/2016/08/26/makefile-c-projects/">A Super-Simple Makefile for Medium-Sized C/C++ Projects</a></li>
    </ol>
  </li>
  <li>C
    <ol>
      <li><a href="https://en.wikipedia.org/wiki/C_POSIX_library">Wikipedia - C POSIX library</a></li>
    </ol>
  </li>
</ul>

  </div>

</article>

<!--
      </div>
    </div>
-->

  </body>

</html>
