编译方法：
1.将makefile的SDK_LIBS,SDK_INCS,UAPI_INCS改成实际project code所在的路径；
2.make clean;make 即可

使用方法：
1.将编译出来的logo和logo.jpg以及lib文件夹拷贝到板子同一个目录；
2.将lib目录export到环境变量：export LD_LIBRARY_PATH=/lib:$PWD/lib:$LD_LIBRARY_PATH
3.运行logo: ./logo

注意事项：
1.demo默认使用的是1024x600的屏参和jpeg图片，如果要换panel对应的屏参和jpeg图片都要替换；
2./config/fbdev.ini里面的FB_WIDTH和FB_HEIGHT需要保持跟屏幕分辨率一致
	FB_WIDTH = 1024
	FB_HEIGHT = 600
