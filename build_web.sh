em++ -o web/index.html src/main.cpp -Wall -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -O3 -Isrc -Lweb -sTOTAL_MEMORY=134217728 -sFORCE_FILESYSTEM=1 -sEXPORTED_RUNTIME_METHODS=ccall -sMINIFY_HTML=0 -sUSE_GLFW=3 -sASYNCIFY --shell-file web/minshell.html web/libraylib.web.a -DPLATFORM_WEB --preload-file resources@resources
cd web
rm *zip
zip -r astescribble.zip index.*

