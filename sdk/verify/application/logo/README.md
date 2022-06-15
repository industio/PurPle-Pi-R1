# 杂项模块 logo 简介
## 功能
+ **生成 fblogo 可执行文件,向framebuffer输出一张图片，例如一张开机logo.期间会初始化disp.**
    -  fblogo 接受 2个参数：1)输出分辨率,2)图片的绝对路径.详情参见fblogo --help
    - 支持LCD/HDMI/VGA 输出
    - 支持jpg/png/raw 3种数据格式,其中png&&raw格式,数据文件的格式必须与fbdev.ini里面设定的颜色格式一致.**jpg格式的图片支持不同的颜色格式自动转换,eg:fbdev.ini里面的颜色是argb1555/rgb565/argb8888,jpeg的格式可以是RGB888**
    - 使用方式：例如 ./fblogo 720p60 /customer/logo.jpg
+ **生成的so/a库提供函数调用方式,参见logo.h(TODO)**

## 编译
+ **默认编译**
    - 进入logo目录,执行sh conf.sh;make clean;make 即可
+ **配置**
    - 打开conf.sh可以看到它支持众多参数：
        + 参数1：DISP_TIMING_TYPE.表示LCD/VGA/HDMI 其中的一种
        + 参数2：DISP_TIMING_LCD_HEADER.表示使用LCD 输出时,对应的屏参文件
        + 举例: 
        >> + VGA输出 执行sh conf.sh VGA
        >> + HDMI输出 执行sh conf.sh HDMI
        >> + LCD输出  执行sh conf.sh LCD SAT070CP50_1024x600.h (需要使用完整头文件名)
        + 其它参数：可以执行 ./configure --help 以供参考.主要提供对应图片格式的支持开关,用以做到最小化可执行文件大小。
            - --enable-png/--disable-png
            - --enable-jpeg/--disable-png
            - --enable-bmp/--disable-bmp
            - --enable-raw/--disable-raw
            - --enable-logodebug /--disable-logodebug
        + 也可以修改conf.sh 里面对应配置项的默认值