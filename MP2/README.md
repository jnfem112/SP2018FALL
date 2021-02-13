
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>SP18MP2</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//mp2">
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
    <h1 id="machine-problem-2---loserfs-easy-peer-to-peer-filesystem">Machine Problem 2 - LoserFS : Easy Peer-to-Peer Filesystem</h1>

<ul>
  <li>Early bird bonus deadline: 11:59 PM, 2018/11/21.</li>
  <li>Final due date: 11:59 PM, 2018/12/05.</li>
  <li>No late submissions.</li>
</ul>

<h2 id="toc">TOC</h2>

<ul id="markdown-toc">
  <li><a href="#toc" id="markdown-toc-toc">TOC</a></li>
  <li><a href="#introduction" id="markdown-toc-introduction">Introduction</a>    <ul>
      <li><a href="#welcomes-loserfs" id="markdown-toc-welcomes-loserfs">Welcomes LoserFS!</a></li>
      <li><a href="#logical-filesystem-view" id="markdown-toc-logical-filesystem-view">Logical filesystem view</a></li>
      <li><a href="#conflict-resolution" id="markdown-toc-conflict-resolution">Conflict resolution</a></li>
      <li><a href="#update-summary" id="markdown-toc-update-summary"><strong>(update)</strong> Summary</a></li>
    </ul>
  </li>
  <li><a href="#required-features" id="markdown-toc-required-features">Required Features</a>    <ul>
      <li><a href="#the-big-picture" id="markdown-toc-the-big-picture">The big picture</a></li>
      <li><a href="#file-hierarchy-and-usage" id="markdown-toc-file-hierarchy-and-usage">File hierarchy and usage</a></li>
      <li><a href="#user-commands" id="markdown-toc-user-commands">User commands</a></li>
      <li><a href="#special-notes" id="markdown-toc-special-notes">Special notes</a></li>
      <li><a href="#bonus-features" id="markdown-toc-bonus-features">Bonus features</a></li>
    </ul>
  </li>
  <li><a href="#judge" id="markdown-toc-judge">Judge</a>    <ul>
      <li><a href="#grading" id="markdown-toc-grading">Grading</a></li>
      <li><a href="#bonus" id="markdown-toc-bonus">Bonus</a></li>
      <li><a href="#guarantees" id="markdown-toc-guarantees">Guarantees</a></li>
      <li><a href="#example-test-cases" id="markdown-toc-example-test-cases">Example test cases</a></li>
    </ul>
  </li>
  <li><a href="#resources" id="markdown-toc-resources">Resources</a></li>
</ul>

<h2 id="introduction">Introduction</h2>

<h3 id="welcomes-loserfs">Welcomes LoserFS!</h3>

<p>We are going to implement a simple peer-to-peer filesystem from scratch. It’s built on top of gangs of losers we made, so let’s call it <strong>LoserFS</strong>. To start off, we have to understand how it works.</p>

<p>We upgrade loser from MP1 to <strong>loser_peer</strong> server at first. It is basically maintains a chain of commits, or <strong>log</strong>, like loser in MP1. Many loser_peers cooperate together to form a <em>single filesystem</em>. <strong>They can look up logs from each other, but can only append new commits to its own log.</strong></p>

<p>But where comes the filesystem? The magic lies in the way loser_peers organize others’ logs into a <em>logical view</em>.</p>

<h3 id="logical-filesystem-view">Logical filesystem view</h3>

<p>Suppose a loser_peer, <em>Resol</em>, has only one best friend, <em>Reep</em>. Let’s look at their logs.</p>

<ul>
  <li><em>Resol</em>’s log
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit 1
[new_file]
love_letter.txt
[modified]
[copied]
[deleted]
(MD5)
love_letter.txt 51dfcbaeb6beab0729f6aca504cc429b
(timestamp)
1541611000000
<br>
# commit 2
[new_file]
QQ.txt
[modified]
[copied]
[deleted]
love_letter.txt
(MD5)
QQ.txt 7e56035a736d269ad670f312496a0846
(timestamp)
1541611500000
</code></pre></div>    </div>
  </li>
  <li><em>Reep</em>’s log
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit 1
[new_file]
thanks_good_guy.txt
[modified]
[copied]
[deleted]
(MD5)
thanks_good_guy.txt 9e204f247de876733be099d5fbfa9ada
(timestamp)
1541611200000
</code></pre></div>    </div>
  </li>
</ul>

<p>The log is similar to <code class="highlighter-rouge">loser log</code> in MP1, except they are arranged in descending order, and have extra <strong>(update)</strong> UNIX <em>(timestamp)</em> in milliseconds field to tell its occurrence. You can notice MD5 is calculated only when that file content is affected in that commit. To form a single filesystem, we <em>replay</em> the all commits ordered by timestamp. The resulting logical view is shown below. We eventually have two files. Note that <code class="highlighter-rouge">love_letter.txt</code> disappears because it’s deleted during replay. The same rule also applies if there are more than two peers.</p>

<ul>
  <li>The logical view
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>QQ.txt
thanks_good_guy.txt
(MD5)
QQ.txt 7e56035a736d269ad670f312496a0846
thanks_good_guy.txt 9e204f247de876733be099d5fbfa9ada
</code></pre></div>    </div>
  </li>
</ul>

<h3 id="conflict-resolution">Conflict resolution</h3>

<p>What if <em>Resol</em> and <em>Reep</em> creates the same file or modify a file at the same time? We have a set of <em>ordering</em> and <em>conflict resolution</em> rules to determine which commit goes first. Checkout this <a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP2/README/RULE.md">document</a> to understand the rules.</p>

<h3 id="update-summary"><strong>(update)</strong> Summary</h3>

<ul>
  <li>Every peer are identical programs, but each of them has a private log.</li>
  <li>One peer can append commits to its private log, but cannot append on others’. However, they can read the logs from others.</li>
  <li>By combining logs from all peers, whoever user on each peer can see identical content of the filesystem.</li>
</ul>

<h2 id="required-features">Required Features</h2>

<h3 id="the-big-picture">The big picture</h3>

<p><img src="https://github.com/jnfem112/Systems-Programming/blob/master/MP2/README/LoserFS.svg" alt="The big picture of LoserFS" /></p>

<p>Our architecture supports arbitrary number of loser_peer processes. They connect to each other over <strong>UNIX domain sockets</strong>. This socket looks like a normal file, and yes, it has a filename, except that input/output to this file is handled by your process. See <a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP2/README/SOCKET.md">here</a> for examples.</p>

<p>In this picture, peer1 owns the socket at <code class="highlighter-rouge">/tmp/mp2-peer1.sock</code>, peer2 owns <code class="highlighter-rouge">/tmp/mp2-peer2.sock</code>, and so on. Peer1 connects to <code class="highlighter-rouge">mp2-peer2.sock</code> and other three sockets. Conversely, peer1 accepts connections from other four peers. In this way, peers talks to each other by sending/receiving bytes. You have to <strong>design the protocol between the peers</strong>.</p>

<p>Every peer provides user command line interface through stdin/stdout. Your program should support <strong>list</strong>, <strong>cp</strong>, <strong>mv</strong> and other several commands. Checkout next section to see details.</p>

<h3 id="file-hierarchy-and-usage">File hierarchy and usage</h3>

<p>Place a <code class="highlighter-rouge">Makefile</code> under our homework directory. The judge will run <code class="highlighter-rouge">make</code> to build a <code class="highlighter-rouge">loser_peer</code> executable in the same directory.</p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>repo
├── MP0
├── MP1
└── MP2
    ├── Makefile
    └── other files
</code></pre></div></div>

<p>The usage is <code class="highlighter-rouge">./loser_peer [CONFIG_FILE]</code>. Our program loads specified config file with this example content.</p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>name = resol
peers = reep hsarc gnah
repo = /home/resol/repo
</code></pre></div></div>

<p>According the the config, it creates a socket at <code class="highlighter-rouge">/tmp/mp2-resol.sock</code> on startup, and stores files and commits in <code class="highlighter-rouge">/home/resol/repo</code> directory.</p>

<ul>
  <li>If the config file is not readable, socket cannot be created, or repo direcotry doesn’t exist or not writable, exit with code <code class="highlighter-rouge">EXIT_FAILURE</code>.</li>
  <li>The program expects <strong>(update, sorry for typo)</strong> <code class="highlighter-rouge">/tmp/mp2-reep.sock</code> and others from peers. If any one of them is missing, the program ignores it and continue to accept user commands.</li>
</ul>

<h3 id="user-commands">User commands</h3>

<p>We use <em>Resol</em> and <em>Reep</em> example above in introduction section for better clarification.</p>

<ul>
  <li>
    <p><strong>list</strong></p>
    <p>This commands shows the <em>most recent</em> logical view. It lists all filenames in dictionary order and their digest. In the <em>Resol</em> and <em>Reep</em> example above, the output should be:</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>QQ.txt
thanks_good_guy.txt
(MD5)
QQ.txt 7e56035a736d269ad670f312496a0846
thanks_good_guy.txt 9e204f247de876733be099d5fbfa9ada
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>history [-a]</strong></p>
    <p>Without <code class="highlighter-rouge">-a</code> option, it shows the commits history of local repository ordered by timestamp. In our example, the output should be the following for peer <em>Resol</em>:</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit 1
[new_file]
love_letter.txt
[modified]
[copied]
[deleted]
(MD5)
love_letter.txt 51dfcbaeb6beab0729f6aca504cc429b
(timestamp)
1541611000000
<br>
# commit 2
[new_file]
QQ.txt
[modified]
[copied]
[deleted]
love_letter.txt
(MD5)
QQ.txt 7e56035a736d269ad670f312496a0846
(timestamp)
1541611500000
</code></pre></div>    </div>
    <p>With <code class="highlighter-rouge">-a</code> option supplied, it prints the <em>merged</em> commit history from all peers. You make to make sure every peer have identical output and obey the ordering rules.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit 1
[new_file]
love_letter.txt
[modified]
[copied]
[deleted]
(MD5)
love_letter.txt 51dfcbaeb6beab0729f6aca504cc429b
(timestamp)
1541611000000
<br>
# commit 2
[new_file]
thanks_good_guy.txt
[modified]
[copied]
[deleted]
(MD5)
thanks_good_guy.txt 9e204f247de876733be099d5fbfa9ada
(timestamp)
1541611200000
<br>
# commit 3
[new_file]
QQ.txt
[modified]
[copied]
[deleted]
love_letter.txt
(MD5)
QQ.txt 7e56035a736d269ad670f312496a0846
(timestamp)
1541611500000
</code></pre></div>    </div>
    <p>Note the format differs from MP1. MD5 digest is calculated only for file creation and modification, and extra <em>(timestamp)</em> in milliseconds is provided. <strong>(update)</strong> The last commit in output does not have extra line breaks.</p>
  </li>
  <li>
    <p><strong>cp SOURCE DEST</strong></p>
    <p>UNIX-style file copy. The <em>SOURCE</em> and <em>DEST</em> paths can be either a local file, such as <code class="highlighter-rouge">/etc/passwd</code>, or a file inside LoserFS, prefixed with @. For example, <code class="highlighter-rouge">cp /etc/fstab @target.txt</code> copies <code class="highlighter-rouge">/etc/fstab</code> from local machine to <code class="highlighter-rouge">target.txt</code> in LoserFS. <code class="highlighter-rouge">cp @myfile.txt /tmp/target.txt</code> does similarly.</p>
    <p>If <em>SOURCE</em> does not exist, print <code class="highlighter-rouge">fail</code> in stdout, otherwise print <code class="highlighter-rouge">success</code> once it completes. <em>DEST</em> file may be replaced if it already exists.</p>
  </li>
  <li>
    <p><strong>mv SOURCE DEST</strong></p>
    <p>UNIX-style file renaming. Like <code class="highlighter-rouge">cp</code> command, we can specify local machine file and LoserFS file in <em>SOURCE</em> and <em>DEST</em>. If <em>SOURCE</em> does not exist or cannot be un, do nothing but print <code class="highlighter-rouge">fail</code> in stdout, otherwise print <code class="highlighter-rouge">success</code> once it completes. <em>DEST</em> file may be replaced if it already exists.</p>
  </li>
  <li>
    <p><strong>rm FILE</strong></p>
    <p>UNIX-style file deletion. This command can ONLY removes files in LoserFS. Hence, FILE is always prefixed with @ like <code class="highlighter-rouge">rm @myfile.txt</code>.</p>
  </li>
  <li>
    <p><strong>exit</strong></p>
    <p>Cause the program print <code class="highlighter-rouge">bye</code> and exit normally. If there’s an ongoing file copy session, the program waits until all sessions completes.</p>
  </li>
</ul>

<h3 id="special-notes">Special notes</h3>

<ul>
  <li>
    <p>Only regular files are considered. Directory support is treated as bonus.</p>
  </li>
  <li>
    <p>Suppose a file is copied from peer A to peer B. If source file is modified or deleted before copying completes, it still proceeds with original content.</p>
  </li>
  <li>
    <p>Be aware of <strong>concurrency</strong>. Commands on peer A should not block user commands on peer B. Also, if multiple peers copy files from A, they should not block each other.</p>
  </li>
  <li>
    <p>Every peer keeps track of others’ logs as soon as possible. Suppose peer A fetches peer B’s log and then B exits. The command <code class="highlighter-rouge">history -a</code> on A  includes latest commits from B. Also, if B never shows up since A’s startup, we pretend B has empty log until B starts.</p>
  </li>
  <li>
    <p><strong>(update)</strong> Users cannot directly make a commit, but peer program decides when and how to make a commit. The only restriction is that every peer can only append new commits to its own log. Be aware of the grading criteria that other peers can see updated log in 0.5 second.</p>
  </li>
  <li>
    <p><strong>(update)</strong> The peer should store data in its repo directory specified in config. One grading item requires peers can restore the log and files after restarting. The homework doesn’t assume the format and structure of repo directory, and the judge will never peek the repo.</p>
  </li>
  <li>
    <p><strong>(update)</strong> Peers can exchange data ONLY through sockets. Direct copy from others’ repo is not allowed.</p>
  </li>
  <li>
    <p><strong>(update 2)</strong> The judge will never touch the repo directory, as well as how you manage your files in repo.</p>
  </li>
</ul>

<h3 id="bonus-features">Bonus features</h3>

<p>These features are optional and impose no penalty on yout grade.</p>

<ul>
  <li>
    <p><strong>Directory support</strong>
This features enable us to specify paths like <code class="highlighter-rouge">@first_dir/second_dir/my_file.txt</code>. To implement this feature, you don’t need to calculate MD5 digest on directories. You have to provide details in <code class="highlighter-rouge">bonus.md</code> on the way you apply or not to apply version control on directories.</p>
  </li>
  <li>
    <p><strong>Alternative to timestamp</strong>
LoserFS depends on timestamps to sort the commits. It’s critically flawed because peers may have different clock time and different ticking rate. Hence, LoserFS is ignorant of the true occurance order of commits. You can describe your solution in <code class="highlighter-rouge">bonus.md</code> and implement the <em>proof-of-concept</em>. The solution should work on mere assumption that peers have their own monotonic clocks.</p>
  </li>
</ul>

<h2 id="judge">Judge</h2>

<h3 id="grading">Grading</h3>

<ul>
  <li>(3pt) <code class="highlighter-rouge">history [-a]</code> and <code class="highlighter-rouge">list</code> with 5 peers in total. Finish both to get full points.</li>
  <li><code class="highlighter-rouge">cp</code> and <code class="highlighter-rouge">mv</code>
    <ul>
      <li>(1pt) <strong>(update 3)</strong> Transfer between LoserFS and local machine by <code class="highlighter-rouge">cp</code> and <code class="highlighter-rouge">mv</code>.</li>
      <li>(1pt) Cross-peer transfer.</li>
      <li>(1pt) After the command completes, other peers see latest log in 0.5 second.</li>
      <li>The points is granted only if both <code class="highlighter-rouge">cp</code> and <code class="highlighter-rouge">mv</code> achieve that item.</li>
    </ul>
  </li>
  <li>(2pt) Concurrent data copy between peers.</li>
  <li>(1pt) <code class="highlighter-rouge">rm</code> command.</li>
  <li>(1pt) Peers can <code class="highlighter-rouge">exit</code> and restart without errors, and restore saved commits.</li>
</ul>

<h3 id="bonus">Bonus</h3>

<ul>
  <li>You can obtain 3-point early bird bonus if
    <ul>
      <li><strong>(update 2)</strong> Local machine to LoserFS transfer through <code class="highlighter-rouge">mv</code> and <code class="highlighter-rouge">cp</code>. (LoserFS to local machine is not needed.)</li>
      <li>Finish <code class="highlighter-rouge">history [-a]</code> and <code class="highlighter-rouge">list</code> with 2 peers in total.</li>
      <li>Create empty <code class="highlighter-rouge">early.md</code> in <code class="highlighter-rouge">mp2</code> directory before early bird due date.</li>
    </ul>
  </li>
  <li>Finish one optional feature mentioned above to get at most 3 points.
    <ul>
      <li>Write bonus features and its details in <code class="highlighter-rouge">bonus.md</code> under <code class="highlighter-rouge">mp2</code> directory.</li>
    </ul>
  </li>
</ul>

<h3 id="guarantees">Guarantees</h3>

<ul>
  <li>Files are at most 2 GB in size.</li>
  <li>Only regular files. No directories, symbolic links and hard links.</li>
  <li>At most 5 peers in total.
 <strong>(update)</strong> Peer names, UNIX socekt file addresses and filenames are up to PATH_MAX in length. <strong>(update 2)</strong> Filenames consist of <a href="https://en.wikipedia.org/wiki/ASCII#Printable_characters">printable ASCII</a> except space characters (space, TAB, etc).</li>
  <li>Each line in config file and command output is ended with <code class="highlighter-rouge">\n</code>.</li>
</ul>

<h3 id="example-test-cases">Example test cases</h3>

<p>TA acknowledges the openess of this spec. Please take a look at <a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP2/README/EXAMPLE.md">examples test cases</a>. Hope it make things more clear.</p>

<h2 id="resources">Resources</h2>

<ul>
  <li>Download the <a href="https://github.com/jnfem112/Systems-Programming/tree/master/MP2/README/sample_code_v1">example code</a> to boostrap your work.</li>
  <li>You can either start from your MP1 or this example <a href="https://github.com/jnfem112/Systems-Programming/tree/master/MP2/README/example_mp1_v1">MP1 code</a>.</li>
  <li>UNIX domain socket <a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP2/README/SOCKET.md">tutorial</a></li>
  <li>Remeber to take a look at <a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP2/README/RULE.md">conflict resolution rules</a></li>
</ul>

  </div>

</article>

<!--
      </div>
    </div>
-->

  </body>

</html>
