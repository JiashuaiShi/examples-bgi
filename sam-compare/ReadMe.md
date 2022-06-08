# sam文件比对程序

### 程序功能：
base.sam query.sam 文件比对，比对同链同序的chr和pos是否相同。
### 实现过程:
Step1: 先读取base.sam文件，根据Qname和正负链标记，设置为key值，把记录作为value值，建立MultiHashMap。   
Step2: 把query.sam的记录和MultiHashMap的记录进行对比。   
Step3: 规则如下：1) 比对相同，则删除原Hash记录。  2) 比对不同，则把不同的行填入MultiHashMap。   
Step4: 最后把MultiMap记录到文件中，则为差异的记录。（另外，Hash查找未命中，是数据源的原因，单独保存未命中的记录。）

