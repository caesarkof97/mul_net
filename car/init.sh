#!/usr/bin/expect 
spawn make clean
spawn make
expect "car.ko" {
	send_user "make success!\n"
	spawn sudo insmod car.ko
	expect "caesar 的密码："
	send "ji\r"
	expect "exists" {
		send_user "file exists!\n"
		exit
	} default {
		send_user "insmod success!\n"
	}
			
} default {
	send_user "make failed !\n"
	exit
}

spawn sudo dmesg -c
expect "caesar 的密码："
send "ji\r"
interact

