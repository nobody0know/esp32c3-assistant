# ESP32C3智能门禁助手

## 硬件开源:

[工程 - 立创开源硬件平台](https://oshwhub.com/nobody_0_knows/esp32c3-intelligent-access-contr)

## 开发环境：

使用VScode + ESP IDF V5.3.1/最新版本开发，参考立创实战派ESP32C3电路和部分代码

## 功能设计:

通过ST的陀螺仪：LSM6DSO的倾斜检测功能，将倾斜检测输出绑定到INT2脚，通过陀螺仪检测开关门的倾斜来实现唤醒ESP32C3并触发迎宾语音，目前按键按下boot按键即可触发音频

使用LVGL V8.3.10和NXP的guiguider设计界面，画了一个查看天气和时间的界面

使用[GPT-SoVITS-v2](https://www.modelscope.cn/studios/xzjosh/GPT-SoVITS-V2) 生成迎宾音频（项目中目前只生成了东雪莲的迎宾音频，需要其他可自行推理生成），写了一个用ffmpeg把mp3音频转pcm文件的脚本，然后软件文件夹中cmakelist中直接链接生成的pcm文件

## 物料成本:

墨水屏链接：[2.9寸墨水屏模块](https://item.taobao.com/item.htm?_u=m2rrb5rt6cc7&id=673989348784&pisk=fXYiPvbmLhSsjjrhtD7s0nTMRJnd55_XuKUAHZBqY9WIGlEt1i52iKYvXIWAoKvDnOIqCNLDKQOx1Eh17qf2epbv6OBvKjv9GrHscNU4ndOrCCBx1EXVKdJccf6A3tvvghhKy4d61Z_VoY3-yvLKnJvg_GzZgy5OaHdaX7R61Z6qeY3-yCwDlELubsWqtw5cGP74ut7EtsC8QZWN0JrFa97V3KWaLM5li55abPrEt_f775z4b95F1_w4uK7qt95m6fqNeE-y8ALOFfZR6GLhsMSp_9uD9e5MbTAaD-y4G1jGUCWneCpN8MRfmENboMdwVK1yQRkGuBfkE_viDS6MYQLOBQV8oC7MoI-VxfuwtNXGcF9iyv_M0dTcfhVjNB7wd365bDMCtFLRmTs3LARptOSkmdHQuOKkTpjJJJUR8Q-h-gk3YulD-r1EMerbcG5CtTHYOlDolg-f_Xc32IsNO_6-tXqbcG5CtThntuLfb615e&spm=a1z09.2.0.0.499c2e8dzxN6Oc&skuId=5023201206441)

项目成本大概120￥，墨水屏模块43.5+ESP32C3模块（不带IFA天线版）13.4+LSM6DSOWTR陀螺仪 6.06+CH343G 3.6 +板载麦克风（暂未开发可不贴）3.3+ES8311 2.9+PCA9557PW 2.64+钽电容1.9+TP5400 1.4 = 95.8￥加上各种杂七杂八的小件，FPC天线也才一块钱一根，合计一百来块不超一百五

## 焊接注意:

建议使用钢网加锡膏进行贴装焊接，淘宝买一个10cm*15cm的钢网，发给他Gerber文件即可，如果怕对位麻烦，边角四个安装孔钢网上也可以打孔

除了电池端子、屏幕模块的排母和开关外其他都可以一起贴装好后上热台

部分电容电阻可能会连锡搭在一起，问题不大，在热台上用镊子轻轻划开即可，小心别碰到其他元件

常连锡大户：TYPE C座子,ES8311,NS4150B,LSM6DSO这些估计得拿烙铁和松香划开，TYPE C我是每回必连的，都要自己划开
