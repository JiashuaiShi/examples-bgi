# sam文件记录查询

### 程序功能：

由于sam文件可能将近20G，采用先读取内存后查询的方式。
读入一对sam文件，查询其中满足Qname的记录。

### 实现过程:

把sam1文件和sam2文件读取到MultiHashMap，查询条件为qname，分别查询两个MultiHashMap中满足查询条件的记录。

