prog_audio_all_test_case使用说明:
-t: 程序的运行时间（秒数），不指定则会一直运行
-I: 使能AI
-o: AI录音的输出路径
-d: AI的设备ID(Amic[0] Dmic[1] I2S RX[2] Linein[3])
-m： AI的声道模式(Mono[0] Stereo[1] Queue[2])
-c: AI通道数
-v: AI音量参数(Amic 0~21, Dmic 0~4, Linein 0~7)
-s: AI采样率8000/16000/32000/48000
-h: 使能AI Hpf
-g: 使能AI Agc
-e: 使能AI Eq
-n: 使能AI NR
-r: AI 重采样采样率8000/16000/32000/48000
-a: AI 编码类型g711a/g711u/g726_16/g726_24/g726_32/g726_40
-A: 使能AED
-b: 使能AEC
-B： 使能Hw loop back AEC
-S: 使能SSL
-F: 使能BF
-M： mic的间距（使能SSL或BF时，必须设置，步长为1cm）
-w: 指定AEC运行的采样率(大部分情况下不需要设置)
-W：开启录制原始的PCM
-C: 指定bf的角度


-O: 使能AO
-i: AO播放的输入文件路径
-D: AO设备ID(Lineout[0] I2S TX[1] HDMI[2] HDMI+Lineout[3])
-V: AO音量参数
-H: 使能AO Hpf
-G: 使能AO Agc
-E: 使能AO Eq
-N: 使能AO NR
-R: AO 重采样采样率8000/16000/32000/48000

