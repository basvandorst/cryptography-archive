PGP 2.6.3i 中文语言模块安装说明
===============================

1) 将LANGUAGE.TXT和ZH.HLP解压到PGP目录中(通常是C:\PGP)。

2) 编辑PGP目录中的CONFIG.TXT文件，并作以下修改：
   Language = en      --->   Language = zh
   CharSet = cp850    --->   CharSet = NoConv
   (注意如果CharSet前有#号的话必须去掉。)
