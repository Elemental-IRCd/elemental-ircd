# Contribution guidelines

Please open contributions as either issue reports or pull requests. We will ask
that anything that requires testing outside our normally supported platforms be
marked as such.

Style
-----

Please run all code against the following `astyle` command before sending in
a pull request:

```console
$ astyle --style=linux --mode=c -n $file
```

It should be trivial to set up your text editor to do this for you. If you use
vim, add this to your vim configuration:

```vimscript
" Lvimrc
" if .lvimrc exists in current or parent directory of the currently loaded file,
" load it as config
if filereadable('../.lvimrc')
  source ../.lvimrc
endif
if filereadable('./.lvimrc')
  source ./.lvimrc
endif
```

Otherwise please make sure the appropriate command is ran as part of your 
editing process before you send a pull request. All pull requests that do not 
follow the coding style will not be considered until they follow the coding 
style.

Testing
-------

When testing Elemental, please be sure to test it on a network of at least 
3 instances of elemental. Please also be sure to have one of these instances 
run **without** the patch you are testing, to be able to show that there is 
a difference.
