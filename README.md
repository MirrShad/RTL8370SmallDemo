# Readme
This is a small demo of RTL8370MB. The Keil project file is in udpbootloader/src2000Proj. 
We not only use RTL8370MB as a switch but also as a PHY chip. If you are only using RTL8370MB as switch, my project won't do any help to you.
In the rar file, I also included the API for RTL8231, because when 8370 is used as a PHY chip, we need to set the strapping of enable LED to disable. So 8370 need to control LED through  8231.
I implemented a bootloader based on UDP, please just ignore the code written for bootloader. As I only deal with 8370 in the beginning to initialize 8370, and the rest is to deal with LWIP, I think extracting code for 8370 won't be a problem, so I just simply upload the whole file.