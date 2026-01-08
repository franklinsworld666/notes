---
title: Vim  
tags:  
notebook: 1102 程序开发 
---
[TOC]

## 移动光标
+ 左移光标: h
+ 下移光标: j
+ 上移光标: k                     
+ 右移光标: l
+ 移动到下个单词开头: w           
+ 移动到下个单词结尾: e           
+ 移动到上个单词开头: b
+ 移动到行首: 0
+ 移动到行首的非空白符: ^
+ 移动到行尾: $           
+ 移动到行内最后一个非空白符: g_
+ 移动到字符 x 下次出现的位置: fx           
+ 移动到字符 x 下次出现位置的前个字符: tx           
+ 移动到文件第一行: gg           
+ 移动到文件最后一行: G           
+ 移动到第5行: :5
+ 跳转到配对的符号: %
+ 移动到下一个段落: }
+ 移动到上一个段落: {
+ 移动屏幕使光标居中: zz           
+ 移动屏幕使光标顶部: zt
+ 向上滚动一屏: Ctrl + b            
+ 向下滚动一屏: Ctrl + f            

## 插入
+ 从光标前开始插入字符: i           
+ 从光标后开始插入字符: a           
+ 从当前单词末尾开始插入: ea           
+ 从行首开始插入字符: I           
+ 从行尾开始插入字符: A           
+ 在当前行之下另起一行插入:o           
+ 在当前行之上另起一行插入:O           
+ 删除光标前一个字符：ctrl-h
+ 删除光标前一个单词：ctrl-w
+ 插入tab：ctrl-t
+ 去除tab: ctrl-d
+ 回到normal模式，一次操作后马上进入插入模式: ctrl-o

## 剪切, 复制, 粘贴
+ 剪切当前字符: x           
+ 复制当前单词: yw           
+ 复制当前行: yy           
+ 复制, 从光标位置到行末: y$           
+ 在光标后粘贴: p           
+ 在光标前粘贴: P           
+ 剪切当前单词: dw           
+ 剪切2个单词: d2w
+ 剪切当前行: dd           
+ 剪切, 从光标位置到行末: D           

## 编辑文本
+ 替换当前字符: r           
+ 删除当前字符并插入: s           
+ 将光标所在的单词删除, 然后插入: ciw           
+ 从光标位置开始, 修改单词: cw           
+ 将光标所在的行删除, 然后插入: cc           
+ 将光标处到行尾删除, 然后插入: C           
+ 将下行合并到当前行, 并在之间插入一个空格: J           
+ 改成小写: gu
+ 改成大写: gU 
+ 撤销: u           
+ 撤销u操作: Ctrl + r
+ 当前行向右缩进：>>
+ 当前行向左缩进：<<
+ indent a block with () or {} (cursor on brace): >%           
+ re-indent a block with () or {} (cursor on brace): <%           
+ go comment: gc, then move h or j

## 查找替换
+ 查找光标所在单词：*
+ 查找: /pattern
+ 正则表达式查找：/\v
+ 向上查找: ?pattern
+ 查找下一个: n           
+ 查找上一个: N           
+ 替换当前行第一个str1 --- :s/str1/str2/
+ 替换当前行所有str1   --- :s/str1/str2/g
+ 替换某几行 ---  :1,5s/old/new/g
+ 替换全部 ---  :%s/old/new/g           
+ 逐个替换 ---  :%s/old/new/gc           

## 可视化选中
+ 选中一个字符：v 
+ 选中一行：V
+ 选中一个矩形：ctrl + v 
+ 选择当前单词: aw           
+ 选择被 () 包裹的区域(含括号): ab           
+ 选择被 {} 包裹的区域(含花括号): aB           
+ a block with <> tags : at          
+ 选择被 () 包裹的区域(不含括号): ib           
+ 选择被 {} 包裹的区域(不含花括号) : iB         
+ inner block with <> tags : it           
+ 切换光标到选择区开头/结尾: o           
+ 切换光标到选择区的角: O           

## 可视化模式下命令
+ 向右缩进: >           
+ 向左缩进: <           
+ 复制: y            
+ 剪切: d           
+ 粘贴: p
+ 改成小写：u
+ 改成大写：U

## Practical Vim
+ 重复上次操作：.
+ 如果有多种操作可以选，优先考虑可以用`.`重复的
+ 查看文件编码：set fileencoding
+ change file encoding: :set fenc=utf-8
+ show file format: set fileformat / set ff  
+ set file format: set ff=unix/dos
+ show in hexadecimal: %!xxd
+ exit hexadecimal mode: %!xxd -r
+ edit in hexadecimal mode: edit the left, then save and exit
+ go out of the brackets: `<ESC>A  or <ESC>fca`