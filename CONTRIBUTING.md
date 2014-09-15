# Contribution guidelines

Please open contributions as either issue reports or pull requests. We will ask 
that anything that requires testing outside our normally supported platforms be 
marked as such.

As ircd is such an ancient project with varying code styles, please try to 
follow the current coding style of the file you are in. If all else fails, 
please use the [Linux Kernel](https://www.kernel.org/doc/Documentation/CodingStyle)
coding style.

Please run all code against the following `astyle` command before sending in 
a pull request:

```console
$ astyle --style=linux --mode=c -n $file
```

It should be trivial to set up your text editor to do this for you.

