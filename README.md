### Simple Linux Shell

#### Features

- [x] **Build-in commands**: exit, pwd, cd, wait

```shell
mysh> cd			# change the working directory to $HOME
mysh> cd [dir]		# change the working directory to specific directory
mysh> pwd			# print working directory
/home/username
mysh> wait			# wait for background jobs to finish
```

- [x] **Externel commands**: cat, echo,  ls, ps, find... Depending on binary files in /bin/

```shell
mysh> echo hello
hello
mysh> ls -a
.  ..  .git  .gitignore  Makefile  README.md  mysh  mysh.c  test-mysh.sh
```



- [x] **Redirection**:  Both input and output redirection

```shell
mysh> ls -la /tmp > output	# output redirection
mysh> cat < mysh.c > tmp.c	# input and output redirection
```

- [x] **Background job**: Use a trailing ampersand `'&'` to create background. Use `wait` to wait for background jobs to finish.

```shell
mysh> ls &
mysh> ps &
mysh> find . -name *.c -print &
mysh> wait
```

- [x] **Batch Mode**:  Read from the batch file back to the user before executing it. Use `.\mysh [batch_file]` . to run batch mode.

For example, the content of `test.sh` is as follows:

```shell
echo hello > tmp
cat tmp
```

```shell
bash> mysh test.sh
echo hello > tmp
cat tmp
hello
```

