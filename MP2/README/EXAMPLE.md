
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>Test cases</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//mp2/test_cases.html">
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
    <h1 id="test-cases">Test cases</h1>

<h2 id="early-bird-test-case">Early bird test case</h2>

<ol>
  <li>
    <p><strong>Resol</strong>: One user starts a peer by <code class="highlighter-rouge">./loser_peer resol.config</code> with this config file. The peer is named <em>resol</em>. It assumes the existences the the repo directory <code class="highlighter-rouge">/home/resol/repo</code>.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> name = resol
 peers = reep
 repo = /home/resol/repo
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Reep</strong>: Another user starts a peer by <code class="highlighter-rouge">./loser_peer reep.config</code> with the config.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> name = reep
 peers = resol
 repo = /home/reep/repo
</code></pre></div>    </div>
  </li>
  <li>
    <p>By staring two peers, we can see socket files in <code class="highlighter-rouge">/tmp</code>.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> $ ls /tmp
 mp2-reep.sock
 mp2-resol.sock
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Resol</strong>: If <em>resol</em> user types <code class="highlighter-rouge">history</code> or <code class="highlighter-rouge">history [-a]</code>. Because no commits are made by <em>Resol</em> or <em>Reep</em>, the output is empty. <em>Reep</em> peer behaves similarly.</p>
  </li>
  <li>
    <p><strong>Resol</strong>: Again, if <em>resol</em> user types <code class="highlighter-rouge">list</code>, no files are present in output but an empty MD5 section.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> (MD5)
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Resol</strong>: The <em>resol</em> user decides to create first file by copying a file into LoserFS by the command <code class="highlighter-rouge">cp /home/resol/source.txt @first.txt</code>. the peer program reads <code class="highlighter-rouge">/home/resol/source.txt</code> on local machine, and make a copy in LoserFS named <code class="highlighter-rouge">first.txt</code>. Once the command finishes, it prints the message:</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> success
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Reep</strong>: At this moment, the <em>reep</em> user types <code class="highlighter-rouge">list</code> and the output goes as follows. It shows the hash <code class="highlighter-rouge">22af645d1859cb5ca6da0c484f1f37ea</code>, and of course, if we calculate the MD5 of <code class="highlighter-rouge">source.txt</code> on <em>Resol</em> machine, it should be the same. The judge also tests if the output is updated in 0.5 second after the copy.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> first.txt
 (MD5)
 first.txt 22af645d1859cb5ca6da0c484f1f37ea
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Resol</strong>: The <em>resol</em> user check out its log by <code class="highlighter-rouge">history</code>. It has at least one commit about creating a file.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> # commit 1
 [new_file]
 first.txt
 [modified]
 [copied]
 [deleted]
 (MD5)
 first.txt 22af645d1859cb5ca6da0c484f1f37ea
 (timestamp)
 1541611000000
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Reep</strong>: If <em>reep</em> user runs <code class="highlighter-rouge">history</code>. Since he or she has do nothing so far, the output is still empty.</p>
  </li>
  <li>
    <p><strong>Resol</strong> and <strong>Reep</strong>: If any of <em>resol</em> and <em>reep</em> user runs <code class="highlighter-rouge">history -a</code> at the same time, it should print combined log from both. Hence, we have</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit 1
[new_file]
first.txt
[modified]
[copied]
[deleted]
(MD5)
first.txt 22af645d1859cb5ca6da0c484f1f37ea
(timestamp)
1541611000000
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Reep</strong>: Suppose <em>reep</em> moves a file to repo by <code class="highlighter-rouge">mv /home/reep/another.txt @second.txt</code>. The peer copies the content of <code class="highlighter-rouge">another.txt</code> from machine to <code class="highlighter-rouge">second.txt</code> in repo. Once the command completes, <code class="highlighter-rouge">another.txt</code> is deleted, and it prints the message.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>success
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Resol</strong> and <strong>Reep</strong>: At this moment, both users can see identical <em>logical view</em> by <code class="highlighter-rouge">list</code>.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>first.txt
second.txt
(MD5)
first.txt 22af645d1859cb5ca6da0c484f1f37ea
second.txt a9f0e61a137d86aa9db53465e0801612
</code></pre></div>    </div>
  </li>
  <li>
    <p>At this moment, the <em>reep’s</em> <code class="highlighter-rouge">history</code> is updated due to new commit, while <em>resol’s</em> <code class="highlighter-rouge">history</code> is unchanged. Both <code class="highlighter-rouge">history -a</code> outputs are identical as follows.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit 1
[new_file]
first.txt
[modified]
[copied]
[deleted]
(MD5)
first.txt 22af645d1859cb5ca6da0c484f1f37ea
(timestamp)
1541611000000
<br>
# commit 2
[new_file]
second.txt
[modified]
[copied]
[deleted]
(MD5)
second.txt a9f0e61a137d86aa9db53465e0801612
(timestamp)
1541611005000
</code></pre></div>    </div>
  </li>
</ol>

<p>In early bird test case, no cross-peer file transfer is needed. They only need to share their logs to setup identical <em>logical view</em>.</p>

<h2 id="cross-peer-file-transfer">Cross-peer file transfer</h2>

<p>This test case continues the early bird test case.</p>

<ol>
  <li>
    <p><strong>Resol</strong>: The <em>resol</em> user decides to get a copy of <code class="highlighter-rouge">second.txt</code> by <code class="highlighter-rouge">cp @second.txt /home/resol/copy_of_second.txt</code>. Because <code class="highlighter-rouge">second.txt</code> is hosted by <em>reep</em>, file transfer between them may occur. It prints <code class="highlighter-rouge">success</code> once it finishes, and the MD5 of <code class="highlighter-rouge">copy_of_second.txt</code> should be the same to that in the log.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> $ md5sum /home/resol/copy_of_second.txt
 a9f0e61a137d86aa9db53465e0801612  /home/resol/copy_of_second.txt
</code></pre></div>    </div>
  </li>
  <li>
    <p>Note that <em>resol</em> does a repo to machine copy. There’s no need to make new commits. Thus, the log is unchaged.</p>
  </li>
  <li>
    <p><strong>Reep</strong>: The <em>reep</em> user now runs <code class="highlighter-rouge">cp @first.txt /home/reep/copy_of_first.txt</code>. Due to concurrency support, the <em>resol</em>  can <code class="highlighter-rouge">rm @first.txt</code> before <em>reep’s</em> cp completes. According to our spec, <em>reep</em> still continues the copy of original content until it completes.</p>
  </li>
  <li>
    <p>Again in 3., even <em>resol</em> user removes <code class="highlighter-rouge">/home/resol/source.txt</code> should not affect <em>reep’s</em> cp, becuase LoserFS already has the data and <code class="highlighter-rouge">source.txt</code> on <em>resol’s</em> machine is irrelevant.</p>
  </li>
  <li>
    <p><strong>Resol</strong>: At this moment, <code class="highlighter-rouge">first.txt</code> is removed by <em>resol</em>. <em>Resol</em> should update its <code class="highlighter-rouge">history</code>. Note that MD5 is not needed for file deletion.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> # commit 1
 [new_file]
 first.txt
 [modified]
 [copied]
 [deleted]
 (MD5)
 first.txt 22af645d1859cb5ca6da0c484f1f37ea
 (timestamp)
 1541611000000
<br>
 # commit 2
 [new_file]
 [modified]
 [copied]
 [deleted]
 first.txt
 (MD5)
 (timestamp)
 1541611010000
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Resol</strong> and <strong>Reep</strong>: Both can see combined log as follows, which commits are ordered by timestamps.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> # commit 1
 [new_file]
 first.txt
 [modified]
 [copied]
 [deleted]
 (MD5)
 first.txt 22af645d1859cb5ca6da0c484f1f37ea
 (timestamp)
 1541611000000
<br>
 # commit 2
 [new_file]
 second.txt
 [modified]
 [copied]
 [deleted]
 (MD5)
 second.txt a9f0e61a137d86aa9db53465e0801612
 (timestamp)
 1541611005000
<br>
 # commit 3
 [new_file]
 [modified]
 [copied]
 [deleted]
 first.txt
 (MD5)
 (timestamp)
 1541611010000
</code></pre></div>    </div>
    <p>In the mean time, the <code class="highlighter-rouge">list</code> on both sides prints</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> second.txt
 (MD5)
 second.txt a9f0e61a137d86aa9db53465e0801612
</code></pre></div>    </div>
  </li>
</ol>

<h2 id="program-restart">Program restart</h2>

<p>This test case continues the previous section.</p>

<ol>
  <li>
    <p><strong>Reep</strong>: The <em>reep</em> user decides to quit by <code class="highlighter-rouge">exit</code> command. If <em>resol</em> is copying a file from <em>reep</em>. The <em>resol</em> peer has to wait until the copy completes.</p>
  </li>
  <li>
    <p><strong>Resol</strong>: After <em>reep</em> exits, <em>resol</em> can still see recent log by <em>history -a</em>. However, if <em>resol</em> user runs <code class="highlighter-rouge">cp @second.txt /home/resol/another_copy.txt</code>, since <code class="highlighter-rouge">second.txt</code> is hosted by <em>reep.txt</em>, the command <strong>may</strong> fail with the message.</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> fail
</code></pre></div>    </div>
    <p>The judge can accept command failure in this case. However, it’s still possible that <em>resol</em> already has a copy and the command suceeds. It’s up to your protocol design. The judge can accept is and will not bother with your implementation.</p>
  </li>
  <li>
    <p><strong>Reep</strong>: Now the <em>reep</em> user restarts the program by <code class="highlighter-rouge">./loser_peer reep.config</code>. It should restore its files and log from repository. Hence, if <em>reep</em> user types <code class="highlighter-rouge">history</code>, he’ll get</p>
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code> # commit 1
 [new_file]
 second.txt
 [modified]
 [copied]
 [deleted]
 (MD5)
 second.txt a9f0e61a137d86aa9db53465e0801612
 (timestamp)
 1541611005000
</code></pre></div>    </div>
  </li>
  <li>
    <p><strong>Reep</strong>: The <em>reep</em> user can continue his or her work. For example, <code class="highlighter-rouge">cp @second.txt @third.txt</code> makes a copy in repositoy. This command affects <em>reep’s</em> private log (<code class="highlighter-rouge">history</code>) and the combined log every peer can see (<code class="highlighter-rouge">history -a</code>).</p>
  </li>
</ol>

  </div>

</article>

<!--
      </div>
    </div>
-->

  </body>

</html>
