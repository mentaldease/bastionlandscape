for /R %%i in (*.c) do svn propset svn:eol-style "native" %%i
for /R %%i in (*.h) do svn propset svn:eol-style "native" %%i
for /R %%i in (*.cpp) do svn propset svn:eol-style "native" %%i
for /R %%i in (*.hpp) do svn propset svn:eol-style "native" %%i
for /R %%i in (*.inl) do svn propset svn:eol-style "native" %%i

