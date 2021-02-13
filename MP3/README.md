
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>SP18MP3</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//mp3">
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
    <h1 id="machine-problem-3---csie-box">Machine Problem 3 - CSIE Box</h1>

<ul>
  <li>Final due date: 11:59 PM, 2019/01/16.</li>
  <li>No late submissions.</li>
</ul>

<h2 id="introduction">Introduction</h2>

<p>In MP3, we want you to practice how to use non-blocking I/O and handling signals.</p>

<h3 id="csie-box">CSIE box</h3>

<p>CSIE box is a client-server service, with one client and one server (you can extend it to support multiple clients and get bouns).
Server and client will both monitor a directory. The files in both side should always be same.
The server and client communicate with each other through FIFO. Both should handle that FIFO may be accidently broken.</p>

<h2 id="required-features">Required Features</h2>

<h3 id="server">Server</h3>
<ol>
  <li>Server will monitor a directory that already exist. Then, it will create FIFOs, and wait for client to connect.</li>
  <li>Server should not crash when client is disconnected.</li>
</ol>

<h3 id="client">Client</h3>
<ol>
  <li>Client will monitor an empty directory. If it is not empty, remove all content.</li>
  <li>Before connect to the Server, the monitored directory’s permission should set to 000 for preventing anyone to write it.</li>
  <li>Set to 700 after synced.</li>
  <li>Client will be terminate by SIGINT (ctrl+c). After the client receive this signal, client should remove the monitored directory.</li>
  <li>Client should not crash when server is disconnected.</li>
</ol>

<h3 id="file-hierarchy-and-usage">File hierarchy and usage</h3>

<p>Place a <code class="highlighter-rouge">Makefile</code> under our homework directory. The judge will run <code class="highlighter-rouge">make</code> to build a <code class="highlighter-rouge">csie_box_server</code> and <code class="highlighter-rouge">csie_box_client</code>
executable in the same directory.</p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>repo
├── MP0
├── MP1
└── MP2
└── MP3
    ├── Makefile
    └── other files
</code></pre></div></div>

<p>We will run the server by <code class="highlighter-rouge">./csie_box_server [CONFIG_FILE]</code>.
Your program should loads the specified config file with this example content.</p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>fifo_path = /home/fifo_dir
directory = /home/resol/dir
</code></pre></div></div>

<p>According the the config, it creates two fifos, <code class="highlighter-rouge">/home/fifo_dir/server_to_client.fifo</code> and <code class="highlighter-rouge">/home/fifo_dir/client_to_server.fifo</code>
on startup, and monitory the files in <code class="highlighter-rouge">/home/resol/dir</code> directory.</p>

<p>We will run the client by <code class="highlighter-rouge">./csie_box_client [CONFIG_FILE]</code>.
The config file’s format is same as the server.</p>

<ul>
  <li>If the config file is not readable, fifo cannot be created, 
exit with code <code class="highlighter-rouge">EXIT_FAILURE</code>.</li>
</ul>

<h3 id="special-notes">Special notes</h3>

<h2 id="judge-and-grading-15pt">Judge and grading (15pt)</h2>

<ol>
  <li>Run <code class="highlighter-rouge">csie_box_server</code>. Check FIFOs exist. (2pt)</li>
  <li>Stop <code class="highlighter-rouge">csie_box_server</code> and run <code class="highlighter-rouge">csie_box_client</code>. Check the permission of monitored directory. (2pt)</li>
  <li>Run <code class="highlighter-rouge">csie_box_server</code>. Check the monitored directory has been synced. (2pt)</li>
  <li>edit the files in both side. Check the contents are always synced. (2pt)</li>
  <li>Stop <code class="highlighter-rouge">csie_box_server</code>. Check <code class="highlighter-rouge">csie_box_client</code> still alive. (1pt)</li>
  <li>Run <code class="highlighter-rouge">csie_box_server</code> and edit the files in both side. Check the contents are always synced. (2pt)</li>
  <li>Stop <code class="highlighter-rouge">csie_box_client</code>. Check the monitored directory has been removed and server still alive. (2pt)</li>
  <li>Run <code class="highlighter-rouge">csie_box_client</code>. Check the monitored directory has been synced with <code class="highlighter-rouge">csie_box_server</code>. (1pt)</li>
  <li>Stop all. Check FIFOs are closed. (1pt)</li>
</ol>

<h3 id="bonus-pt">Bonus (pt)</h3>

<ul>
  <li>Also sync the file infos (permission, atime, ctime, mtime) (1pt)</li>
  <li>Support 3 clients. (2pt)</li>
  <li>Support directory removal. (1pt)</li>
</ul>

<p>Write a readme file if you have implemented any of them.</p>

<h3 id="guarantees">Guarantees</h3>

<ul>
  <li>fifo_path is an empty directory.</li>
  <li>The number of subdirectory in the monitored directory will be less than 10.</li>
  <li>The directory will not be removed during monitoring.</li>
  <li>Each file will be less than 10MB.</li>
  <li>Interval between any operation or query is greater than one second.</li>
</ul>

<h2 id="functions-you-may-need">Functions you may need</h2>

<ul>
  <li>inotify</li>
  <li>select</li>
  <li>fifo</li>
  <li>signal</li>
</ul>

<h2 id="tips-for-workstation-usage">Tips for Workstation Usage</h2>
<p>https://wslab.csie.ntu.edu.tw/tips_for_workstation_usage.html</p>

<h2 id="simple-judge">Simple Judge</h2>
<p><a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP3/judge.py">judge.py</a>
This is a simple judge program. You can use it to test your code.
It cover all judge steps with a small test case without bonus.</p>

<h3 id="usage">Usage</h3>
<ul>
  <li>Put the judge.py into your MP3 directory</li>
  <li>Run <code class="highlighter-rouge">python3 judge.py</code></li>
</ul>

<h2 id="faq">FAQ</h2>

<ol>
  <li>Create directory with permission 000.
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>mkdir("xddir", 0);
</code></pre></div>    </div>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>d---------  2 newslab newslab 4096 Dec 19 04:18 xddir
</code></pre></div>    </div>
  </li>
  <li>The subdirectory can be create in a subdirectory.</li>
</ol>

  </div>

</article>

<!--
      </div>
    </div>
-->

  </body>

</html>
