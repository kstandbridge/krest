# krest

This is to demonstrate how to use kengine to create a http server that runs in a console.

```
git clone --recursive https://github.com/kstandbridge/http_spammer.git
build.bat build_dependencies
build.bat
```

At a high level, the win32 executable will load a dll and call into functions created in main.c which means you should only have to compile the win32 layer once. 

You can recompile main.c while the console application is running. It should automatically reload dll.
