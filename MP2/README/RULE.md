
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>Conflict resolution</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//mp2/conflict_resolution.html">
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
    <h1 id="conflict-resolution">Conflict resolution</h1>

<h2 id="commit-ordering-rules">Commit ordering rules</h2>

<p>Building a logical view is equivalent to merging several chains of commits into one unified log. The ordering of commits obeys these rules.</p>

<ul>
  <li>Commits with larger timestamps goes after those with smaller timestamps.</li>
  <li>If one commit goes before another commit in one peer, their order cannot be revered.</li>
  <li>If timestamps are indistinguishable, merge these commits into one using <em>commit merging rules</em>.</li>
</ul>

<h2 id="commit-merging-rules">Commit merging rules</h2>

<ul>
  <li>Apply operations in this order: copy, creation, modification, and then deletion.</li>
  <li>If duplicated creations occur, the one from smallest md5 hash (in dictionary order) takes effect. This rule applies to modification and copy.</li>
</ul>

<p>Suppose <em>Resol</em> and <em>Reep</em> have commits with identical timestamps.</p>

<ul>
  <li><em>Resol</em>’s log
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit X
[new_file]
both_create.txt
create_and_delete.txt
[modified]
both_modify.txt
[copied]
[deleted]
(MD5)
both_create.txt 81dfcbafb6beab0729f6a6a504cc429b
both_modify.txt 60b725f10c9c85c70d97880dfe8191b3
create_and_delete.txt 3b5d5c3712955042212316173ccf37be
(timestamp)
1541611000000
</code></pre></div>    </div>
  </li>
  <li><em>Reep</em>’s log
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit Y
[new_file]
both_create.txt
[modified]
both_modify.txt
[copied]
from.txt =&gt; to.txt
[deleted]
create_and_delete.txt
(MD5)
both_create.txt b924cc405aca6f9270baeb6beabcfd15
both_modify.txt 2cd6ee2c70b0bde53fbe6cac3c8b8bb1
(timestamp)
1541611000000
</code></pre></div>    </div>
  </li>
</ul>

<p>By merging these commits during replay, it results to the commit:</p>

<ul>
  <li>Merged commit
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit Z
[new_file]
both_create.txt
create_and_delete.txt
[modified]
both_modify.txt
[copied]
from.txt =&gt; to.txt
[deleted]
create_and_delete.txt
(MD5)
both_create.txt 81dfcbafb6beab0729f6a6a504cc429b
both_modify.txt 2cd6ee2c70b0bde53fbe6cac3c8b8bb1
create_and_delete.txt 3b5d5c3712955042212316173ccf37be
(timestamp)
1541611000000
</code></pre></div>    </div>
    <p><strong>(Update 4)</strong> In the logical view, we have <code class="highlighter-rouge">both_create.txt</code> from <em>Resol</em>, <code class="highlighter-rouge">both_modify.txt</code> from <em>Reep</em>, copied file <code class="highlighter-rouge">to.txt</code>, and original <code class="highlighter-rouge">from.txt</code>. The <code class="highlighter-rouge">create_and_delete.txt</code> file never appears in the filesystem.</p>
  </li>
</ul>

<h2 id="conflict-resolution-1">Conflict resolution</h2>

<p>Operations in any of these cases don’t take effect.</p>

<ul>
  <li>Creation on an existing file</li>
  <li>Deletion or modification on a non-existing file</li>
  <li>Copying from a non-existing file</li>
</ul>

<p>Note that copying to a existing file can replace that file.</p>

<p>For example, <em>Resol</em> creates <code class="highlighter-rouge">openme.txt</code> and then modifies it, and <em>Reep</em> deletes it before <em>Resol</em>’s modification. According to our rule, <em>Resol</em>’s modification has no effect.</p>

<ul>
  <li><em>Resol</em>’s log
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit 1
[new_file]
openme.txt
[modified]
[copied]
[deleted]
(MD5)
openme.txt 5c5602054a9f08648bc18189b2310f93
(timestamp)
1541611000000
<br>
# commit 2
[new_file]
[modified]
openme.txt
[copied]
[deleted]
(MD5)
openme.txt 39f0132b98181cb84680f9a4502065c5
(timestamp)
1541611050000
</code></pre></div>    </div>
  </li>
  <li><em>Reep</em>’s log
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit 1
[new_file]
[modified]
[copied]
[deleted]
openme.txt
(MD5)
(timestamp)
1541611030000
</code></pre></div>    </div>
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
