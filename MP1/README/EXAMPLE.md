
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>MP1 範例</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//mp1/EXAMPLE.html">
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
    <h1 id="mp1-範例">MP1 範例</h1>

<h2 id="測資範例">測資範例</h2>
<p><a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP1/README/test_1.zip">test_1</a></p>

<p>將測資下載並解壓縮後放到跟loser同資料夾
目錄結構長這樣：</p>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>.
├── loser
└── test_1
    ├── .loser_record
    ├── b
    ├── c
    ├── d
    ├── e
    └── f
</code></pre></div></div>
<p>.loser_record 裡已存有兩次commit
我們先印出log</p>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>&gt;./loser log 2 test_1/
# commit 2
[new_file]
b
[modified]
a
[copied]
a =&gt; c
a =&gt; d
[deleted]
(MD5)
a 2cd6ee2c70b0bde53fbe6cac3c8b8bb1
b 3b5d5c3712955042212316173ccf37be
c 60b725f10c9c85c70d97880dfe8191b3
d 60b725f10c9c85c70d97880dfe8191b3
<br>
# commit 1
[new_file]
a
[modified]
[copied]
[deleted]
(MD5)
a 60b725f10c9c85c70d97880dfe8191b3
</code></pre></div></div>
<p>最後一次的 commit 有4個檔案 a, b, c, d 其中 b 是新檔案， c, d 是從 a 複製過來，而複製完後 a 就被修改了。
最後一次 commit 之後，新增了 e 檔案，修改了 b 檔案，從 c 複製出 f ，並把 a 刪除。</p>

<p>範例輸出導到檔案應該要與此檔案相同
<a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP1/README/example_log">Example of log</a></p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>&gt;./loser status test_1/
[new_file]
e
[modified]
b
[copied]
c =&gt; f
[deleted]
a
</code></pre></div></div>
<p>範例輸出導到檔案應該要與此檔案相同
<a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP1/README/example_status">Example of status</a></p>

<p>.loser_record 的 binery format:</p>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>&gt;hexdump  -x test_1/.loser_record
0000000    0001    0000    0001    0000    0001    0000    0000    0000
0000010    0000    0000    0000    0000    0030    0000    6101    6101
0000020    b760    f125    9c0c    c785    970d    0d88    81fe    b391
0000030    0002    0000    0004    0000    0001    0000    0001    0000
0000040    0002    0000    0000    0000    0070    0000    6201    6101
0000050    6101    6301    6101    6401    6101    d62c    2cee    b070
0000060    e5bd    be3f    ac6c    8b3c    b18b    6201    5d3b    375c
0000070    9512    4250    2321    1716    cf3c    be37    6301    b760
0000080    f125    9c0c    c785    970d    0d88    81fe    b391    6401
0000090    b760    f125    9c0c    c785    970d    0d88    81fe    b391
00000a0
</code></pre></div></div>
<h2 id="大測試資料">大測試資料</h2>

<p><a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP1/README/test_max.zip">test_max</a></p>

<p>此測試資料中含一個 commit。該次 commit 有 1000 個檔案。
然後資料夾中有 1000 個新的檔案，舊的檔案全數刪除，每個檔案皆為 100kB。</p>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>&gt;./loser status test_max/
[new_file]
00000...00001000
00000...00001001
00000...00001002
...
00000...00001998
00000...00001999
[modified]
[copied]
[deleted]
00000...00000000
00000...00000001
00000...00000002
...
00000...00000998
00000...00000999
</code></pre></div></div>

  </div>

</article>

<!--
      </div>
    </div>
-->

  </body>

</html>
