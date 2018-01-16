#!/usr/bin/expect

spawn sudo ifconfig vnet down
expect { 	
	"caesar*的密码：" {
		send "ji\r"
	    expect {
			"没有那个设备" {
				send_user "error, vnet exiting now!\n"
				exit
			}
			default {
				send_user "close vnet success!\n"
			}
		}
	}
}

spawn sudo rmmod vnet
expect { 	
	"caesar*的密码：" {
		send "ji\r"
	    expect {
			"ERROR" {
				send_user "error, vnet exiting now!\n"
				exit
			}
			default {
				send_user "remove mod vnet success!\n"
			}
		}
	}
}
		

spawn sudo dmesg -c
expect "caesar*的密码："
send "ji\r"
expect "vnet stop" {
	send_user "vnet leaving now......\n"
} default {
	send_user "module vnet not exit\n"
}
interact
