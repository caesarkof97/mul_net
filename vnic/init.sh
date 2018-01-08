#!/usr/bin/expect 
spawn make clean
spawn make
expect "vnet.ko" {
	send_user "make vnet success!\n"
	spawn sudo insmod vnet.ko
	expect "caesar 的密码："
	send "ji\r"
	expect "exists" {
		send_user "file vnet exists!\n"
		exit
	} default {
		send_user "insmod vnet success!\n"
	}
			
} default {
	send_user "make vnet failed !\n"
	exit
}

spawn sudo ifconfig vnet 192.168.3.1
	expect "caesar 的密码："
	send "ji\r"
	expect "not found" {
		send_user "no device vnet!\n"
		exit
	} default {
		send_user "config success!\nvnet: 192.168.3.1\n"
	}


spawn sudo dmesg -c
expect "caesar 的密码："
send "ji\r"


interact

