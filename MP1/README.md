
<!DOCTYPE html>
<html>

  <head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">

<!--
  <title>SP18MP1</title>
-->

  <link rel="stylesheet" href="/css/main.css">
  <link rel="canonical" href="https://systemprogrammingatntu.github.io//mp1/">
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
    <h1 id="machine-problem-1---使用-md5-進行版本控制">Machine Problem 1 - 使用 MD5 進行版本控制</h1>

<ul>
  <li>Due Date: 11:59 PM, 2018/11/7 本次作業不允許遲交。</li>
</ul>

<hr />

<h2 id="程式名稱">程式名稱</h2>
<p>我們將在本作業使用目前上課學到的系統呼叫（read、write、lseek 等等）來實作一個 git 的超級弱化版。這支版本控制的程式該取什麼命字才好呢？<a href="https://zh.wikipedia.org/wiki/Git#.E5.91.BD.E5.90.8D.E6.9D.A5.E6.BA.90">維基百科</a>告訴我們原來 git 是 Linus 的自嘲，其意義是「混帳」。但助教連當混帳都不配，就是個人生的輸家，所以我們這次編譯出來的程式請命名爲 “loser” 。</p>

<h2 id="功能">功能</h2>

<p>loser 支援三個子指令（都類似於 git ）： status(顯示目前的修改), commit(提交目前的修改), log(顯示過去提交的歷史) 。</p>

<p>不同於 git 能夠記錄過往的所有歷史而擁有自由自在回到任一 commit 的能力， loser 只能夠知道某些檔案曾被修改過而已。</p>

<p>loser 的運作原理十分簡單，它會將過去所有訊息記錄於所求目錄下的 .loser_record 檔案，所有的子指令都離不開這個檔案，每次 commit 會新增一組記錄，而 status 會去讀取最後一條 commit 記錄以與當前狀態進行比對，log 則根據要求輸出 ./loser_record 的資料。</p>

<p>但是因爲我們不用像 git 一樣需要回到過去的 commit ，記錄過往的所有資料太浪費空間，我們只需要比較過去檔案的 <a href="https://zh.wikipedia.org/wiki/MD5">MD5</a> 摘要即可知道是否產生過變化。</p>

<h3 id="loser_record">.loser_record</h3>

<p>.loser_record 有一項特質，作爲一種元數據，它<strong>不會記錄自身的修改</strong>，也就是說雖然執行 <code class="highlighter-rouge">./loser commit</code> 就可能會改變 .loser_record ，仍舊無需記錄它發生了什麼改變，也無需計算它的 MD5。</p>

<p>本次作業限定以 Bainry I/O 對 .loser_record 進行操作，且 .loser_record 必須依照此處所規定的格式：</p>

<p><img src="https://github.com/jnfem112/Systems-Programming/blob/master/MP1/README/loser_record.png" alt="commit struct" /></p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>其中各項意義為：
    1. number of commit : 代表是第幾次 commit，由 1 開始計算，大小為 4 bytes unsigned integer (uint32_t)
    <br>
    2. number of file : 代表某個給定目錄路徑底下有多少個檔案，大小為 4 bytes unsigned integer (uint32_t)
    <br>
    3. number of add : 代表相對於前一次 commit ，有多少個之前不存在的檔案，大小為 4 bytes unsigned integer (uint32_t)
    <br>
    4. number of modify : 代表相對於前一次 commit ，有多少個之前存在但被修改過的檔案，大小為 4 bytes unsigned integer (uint32_t)
    <br>
    5. number of copy : 代表有多少個檔案是由前一次 commit 中紀錄的檔案複製而來，大小為 4 bytes unsigned integer (uint32_t)
    <br>
    6. number of delete : 代表相對於前一次commit，有多少個之前存在，但現在不存在的檔案，大小為 4 bytes unsigned integer (uint32_t)
    <br>
    7. commit size : 整個 commit 所佔用的 byte 數目，大小為 4 bytes unsigned integer (uint32_t)
    <br>
    接下來，分別對於 n 個 add/modify/copy/delete 以字典順序紀錄 n 筆 檔名長度與檔名，其格式為
        file name size(uint8_t)，file name(&lt; 255 bytes)
    要注意的是，copy 項目須依序紀錄複製來源及複製結果兩者的長度及檔名
    <br>
    最後，要以字典順序記錄給定目錄路徑下所有的檔案名稱及其 md5 ，其格式為
        fiil name size(uint_8)，file name(&lt; 255 bytes)，md5 (16 bytes)
</code></pre></div></div>

<h3 id="子指令">子指令</h3>

<p>loser 支援三個子指令</p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>loser status &lt;目錄&gt;
loser commit &lt;目錄&gt;
loser log &lt;數量&gt; &lt;目錄&gt;
</code></pre></div></div>

<h3 id="loser-status">loser status</h3>

<h4 id="描述">描述</h4>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>loser status &lt;目錄&gt;
</code></pre></div></div>
<p><code class="highlighter-rouge">loser status</code> 是最簡單也最常用的子指令，它能夠追蹤並顯示所求目錄相較於<strong>上一次 commit （不需考慮更久之前的 commit）</strong> 產生了哪些變化，我們將這些變化分類爲：</p>

<ol>
  <li>new_file： 檔名在上一次 commit 沒有出現過，並且不符合copied的條件。</li>
  <li>modified： 檔名在上一次 commit 中出現過，但 MD5 的結果與上次不同。</li>
  <li>copied： 檔名在上一次 commit 沒有出現，但 MD5 的結果與上次 commit 中的某個檔案相同。</li>
  <li>deleted : 檔名在上一次 commit 中出現過，但 當前目錄中已不存在。</li>
</ol>

<p>具體的輸出格式其實就是一個 commit 記錄去掉各項 size 與 MD5 部分：</p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>[new_file]
&lt;新檔案檔名1&gt;
&lt;新檔案檔名2&gt;
.
.
[modified]
&lt;被修改檔案檔名1&gt;
&lt;被修改檔案檔名2&gt;
.
.
[copied]
&lt;原檔名1&gt; =&gt; &lt;新檔名1&gt;
&lt;原檔名2&gt; =&gt; &lt;新檔名2&gt;
.
.
[deleted]
&lt;被刪除檔案檔名1&gt;
&lt;被刪除檔案檔名2&gt;
</code></pre></div></div>

<p>我們特別來看一下[copied]：</p>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>[copied]
&lt;原檔名1&gt; =&gt; &lt;新檔名1&gt;
&lt;原檔名2&gt; =&gt; &lt;新檔名2&gt;
.
.
</code></pre></div></div>
<p>很可能會發生一個檔案的 MD5 與上一次 commit 中不只一個檔案的 MD5 相同的情形，那此時的<原檔名>請使用字典順序最小者。</原檔名></p>

<p>同樣記得[new_file]、[modified]、[copied] 以下都會跟隨一行行字串，每一行都該<strong>遵循字典順序由小到大排序</strong>。</p>

<h4 id="特殊情形">特殊情形</h4>

<ul>
  <li>.loser_record 檔案不存在：視所有檔案爲新檔案</li>
  <li>檔案與上一次 commit 沒有任何不同：輸出
    <div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>  [new_file]
  [modified]
  [copied]
  [deleted]
</code></pre></div>    </div>
  </li>
</ul>

<h3 id="loser-commit">loser commit</h3>

<h4 id="描述-1">描述</h4>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>loser commit &lt;目錄&gt;
</code></pre></div></div>
<p>每次執行會計算與上次 commit 的差異並追加到 .loser_record 檔案的末尾。</p>

<p>格式見 <a href="#.loser_record">.loser_record</a> 一節。</p>

<p>MD5 部分則記錄<strong>目錄底下的所有檔案</strong>（.loser_record 除外）與其 MD5 的對應。</p>

<h4 id="特殊情形-1">特殊情形</h4>

<ul>
  <li>.loser_record 檔案不存在、但存在其他檔案：建立 .loser_record 檔案（權限爲該檔案擁有者可讀可寫），並視所有檔案爲新檔案</li>
</ul>

<h3 id="loser-log">loser log</h3>

<h4 id="描述-2">描述</h4>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>loser log &lt;數量&gt; &lt;目錄&gt;
</code></pre></div></div>
<p>log 子指令會接一個 &lt;數量&gt; ，表示該輸出 .loser_record 檔案中倒數 &lt;數量&gt; 個 commit 資訊。 這些資訊與 .loser_record 檔案記錄的資訊完全相同，但順序相反，換句話說，越新的 commit 越先輸出。</p>

<p>&lt;數量&gt;保證爲一個數字</p>

<p>具體輸出格式則類似 status，只是須註明為第幾次commit，每筆 commit 之間需空格一行。</p>
<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code># commit n
[new_file]
&lt;新檔案檔名1&gt;
&lt;新檔案檔名2&gt;
.
.
[modified]
&lt;被修改檔案檔名1&gt;
&lt;被修改檔案檔名2&gt;
.
.
[copied]
&lt;原檔名1&gt; =&gt; &lt;新檔名1&gt;
&lt;原檔名2&gt; =&gt; &lt;新檔名2&gt;
.
.
[deleted]
&lt;被刪除檔案檔名1&gt;
&lt;被刪除檔案檔名2&gt;
.
.
(MD5)
&lt;檔名&gt; &lt;md5&gt;
.
.
<br>
# commit n-1
.
.
.
</code></pre></div></div>

<h4 id="特殊情形-2">特殊情形</h4>

<ul>
  <li>.loser_record 檔案不存在（.loser_record 只要存在就至少包含一個 commit） ：不輸出任何東西</li>
  <li><code class="highlighter-rouge">loser log &lt;數量&gt; &lt;目錄&gt;</code> 的數量大於目前 commit 的數量：輸出所有歷史</li>
</ul>

<h2 id="更多範例">更多範例</h2>

<p>參考<a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP1/README/EXAMPLE.md">這裡</a></p>

<h2 id="實作細節">實作細節</h2>

<h3 id="輸入輸出">輸入輸出</h3>

<p><code class="highlighter-rouge">./loser status</code>、<code class="highlighter-rouge">./loser log</code> 都寫至標準輸出，<code class="highlighter-rouge">./loser commit</code> 寫至 .loser_record。</p>

<p>測試時也會檢查標準輸出與 .loser_record，以及是否有其他檔案受到修改，但標準錯誤輸出可以自己拿去印東西來協助除錯。</p>

<h3 id="讀取目錄">讀取目錄</h3>

<p>參考連結 <a href="http://man7.org/linux/man-pages/man3/scandir.3.html">Linux Programmer’s Manual</a></p>

<h3 id="md5">MD5</h3>

<p>演算法請參考<a href="https://zh.wikipedia.org/wiki/MD5">維基百科</a>。</p>

<p>小提醒是如果不確定 <code class="highlighter-rouge">unsigned int</code> 到底是多少位元，可以 <code class="highlighter-rouge">#include &lt;stdint.h&gt;</code> 來取用 <code class="highlighter-rouge">uint32_t</code> 跟 <code class="highlighter-rouge">uint64_t</code> 來明確使用 32 位元跟 64 位元的資料型別。</p>

<p>又爲降低同學的負擔， MD5 可以使用上網搜尋程式碼來參考(google “md5 c”)，但請自行驗證其正確性並承擔風險。</p>

<h2 id="評分">評分</h2>

<p>共 10 分</p>
<ol>
  <li>(4 分) <code class="highlighter-rouge">loser status</code> 輸出正確狀態</li>
  <li>(4 分) <code class="highlighter-rouge">loser commit</code> 正確寫入 .loser_record</li>
  <li>(2 分) <code class="highlighter-rouge">loser log</code> 輸出正確歷史資訊</li>
</ol>

<p>注意到 <code class="highlighter-rouge">loser status</code>、<code class="highlighter-rouge">loser commit</code>、<code class="highlighter-rouge">loser log</code>三個指令可以完全讀立運作，因此測試一個子指令時不會測試到其他子指令。同學也可以僅實作部分功能以獲得部分分數。</p>

<h3 id="測試資料的保證">測試資料的保證</h3>

<ul>
  <li>.loser_record 若存在必符合格式並可寫</li>
  <li>.loser_record 檔案小於 2GB</li>
  <li>各子指令參數的<目錄>必存在且可讀可入</目錄></li>
  <li>作用目錄不包含任何目錄，只包含檔案</li>
  <li>所有檔案的名稱皆由英文字母大小寫、數字、底線(_)、點(.)組成，並且長度小於 255 位元組</li>
  <li>作用目錄底下不超過 1000 個檔案</li>
  <li>作用目錄底下所有檔案皆可讀</li>
  <li>作用目錄底下檔案大小總和不超過 100MB</li>
  <li><code class="highlighter-rouge">loser log</code> 輸出的資料量小於 10MB</li>
</ul>

<h3 id="時間限制">時間限制</h3>

<p>任何一個子指令操作時間必須小於 10 秒。</p>

<h2 id="繳交方式">繳交方式</h2>

<p>與 MP0 雷同，請在同一個 repo 下建立另一個 MP1 ，將 Makefile 與程式碼置於其中。</p>

<div class="highlighter-rouge"><div class="highlight"><pre class="highlight"><code>repo
├── MP0
│   ├── Makefile
│   └── other files
└── MP1
    ├── Makefile
    └── other files
</code></pre></div></div>
<p>我們會在 MP1 目錄下執行 <code class="highlighter-rouge">make</code> ，並測試此指令生成的 loser 執行檔。</p>

<h2 id="有用連結">有用連結</h2>

<ul>
  <li><a href="https://zh.wikipedia.org/wiki/MD5">MD5 - 維基百科，自由的百科全書</a></li>
  <li><a href="https://www.byvoid.com/zht/blog/cpp-fstream">c++ binary io</a></li>
  <li><a href="https://github.com/jnfem112/Systems-Programming/blob/master/MP1/README/EXAMPLE.md">範例頁</a></li>
  <li><a href="https://zh.wikipedia.org/wiki/字节序">endian</a></li>
</ul>

<h2 id="可能用到的指令">可能用到的指令</h2>

<ul>
  <li>open, fopen</li>
  <li>read, fread</li>
  <li>write, fwrite</li>
  <li>lseek, fseek</li>
  <li>opendir, readdir, scandir</li>
  <li>memcmp, strcmp</li>
</ul>

  </div>

</article>

<!--
      </div>
    </div>
-->

  </body>

</html>
