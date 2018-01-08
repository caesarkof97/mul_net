#!/usr/bin/expect
spawn sudo rmmod car
expect { 	
	"caesar*的密码：" {
		send "ji\r"
	    expect {
			"ERROR" {
				send_user "error, exiting now!\n"
				exit
			}
			default {
				send_user "remove mod success!\n"
			}
		}
	}
}
		

spawn sudo dmesg -c
expect "caesar*的密码："
send "ji\r"
expect "car stop" {
	send_user "leaving now......\n"
} default {
	send_user "module car not exit\n"
}
interact
