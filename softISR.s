	.file	"softISR.c"
	.text
.Ltext0:
	.comm	a,4,4
	.comm	b,4,4
	.comm	p,8,8
	.comm	jump_destination,200,32
	.section	.rodata
	.align 8
.LC0:
	.string	"\nError dereferencing pointer and we're in interrupt handler"
	.text
	.globl	exception_handler
	.type	exception_handler, @function
exception_handler:
.LFB0:
	.file 1 "softISR.c"
	.loc 1 8 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	.loc 1 9 0
	movl	$.LC0, %edi
	call	puts
	.loc 1 10 0
	movq	$b, p(%rip)
	.loc 1 11 0
	movl	$1, %esi
	movl	$jump_destination, %edi
	call	longjmp
	.cfi_endproc
.LFE0:
	.size	exception_handler, .-exception_handler
	.section	.rodata
	.align 8
.LC1:
	.string	"Addr of interrupt handler -> %8.8X \n"
	.align 8
.LC2:
	.string	"Trying to dereference pointer p with value ->  "
.LC3:
	.string	"%8.8X .\n "
	.align 8
.LC4:
	.string	"After dereferencing pointer, its value is: %d\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB1:
	.loc 1 15 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 1 18 0
	movl	$exception_handler, %esi
	movl	$11, %edi
	call	signal
	.loc 1 19 0
	movl	$3, b(%rip)
	.loc 1 20 0
	movq	$0, p(%rip)
	.loc 1 22 0
	movl	$exception_handler, %esi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf
	.loc 1 24 0
	movl	$jump_destination, %edi
	call	_setjmp
	.loc 1 25 0
	movl	$.LC2, %edi
	movl	$0, %eax
	call	printf
	.loc 1 26 0
	movq	p(%rip), %rax
	movl	(%rax), %eax
	movl	%eax, %esi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	.loc 1 27 0
	movq	p(%rip), %rax
	movl	(%rax), %eax
	movl	%eax, %esi
	movl	$.LC4, %edi
	movl	$0, %eax
	call	printf
	.loc 1 28 0
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	main, .-main
.Letext0:
	.file 2 "/usr/include/x86_64-linux-gnu/bits/sigset.h"
	.file 3 "/usr/include/x86_64-linux-gnu/bits/setjmp.h"
	.file 4 "/usr/include/setjmp.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x1bb
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF21
	.byte	0xc
	.long	.LASF22
	.long	.LASF23
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF1
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF2
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF4
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.long	.LASF5
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF6
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF7
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF8
	.uleb128 0x4
	.byte	0x80
	.byte	0x2
	.byte	0x1b
	.long	0x88
	.uleb128 0x5
	.long	.LASF13
	.byte	0x2
	.byte	0x1d
	.long	0x88
	.byte	0
	.byte	0
	.uleb128 0x6
	.long	0x2d
	.long	0x98
	.uleb128 0x7
	.long	0x65
	.byte	0xf
	.byte	0
	.uleb128 0x8
	.long	.LASF11
	.byte	0x2
	.byte	0x1e
	.long	0x73
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF9
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF10
	.uleb128 0x8
	.long	.LASF12
	.byte	0x3
	.byte	0x1f
	.long	0xbc
	.uleb128 0x6
	.long	0x5e
	.long	0xcc
	.uleb128 0x7
	.long	0x65
	.byte	0x7
	.byte	0
	.uleb128 0x9
	.long	.LASF24
	.byte	0xc8
	.byte	0x4
	.byte	0x22
	.long	0xfd
	.uleb128 0x5
	.long	.LASF14
	.byte	0x4
	.byte	0x28
	.long	0xb1
	.byte	0
	.uleb128 0x5
	.long	.LASF15
	.byte	0x4
	.byte	0x29
	.long	0x57
	.byte	0x40
	.uleb128 0x5
	.long	.LASF16
	.byte	0x4
	.byte	0x2a
	.long	0x98
	.byte	0x48
	.byte	0
	.uleb128 0x8
	.long	.LASF17
	.byte	0x4
	.byte	0x30
	.long	0x108
	.uleb128 0x6
	.long	0xcc
	.long	0x118
	.uleb128 0x7
	.long	0x65
	.byte	0
	.byte	0
	.uleb128 0xa
	.long	.LASF18
	.byte	0x1
	.byte	0x7
	.quad	.LFB0
	.quad	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.long	0x143
	.uleb128 0xb
	.string	"sg"
	.byte	0x1
	.byte	0x7
	.long	0x57
	.uleb128 0x2
	.byte	0x91
	.sleb128 -20
	.byte	0
	.uleb128 0xa
	.long	.LASF19
	.byte	0x1
	.byte	0xe
	.quad	.LFB1
	.quad	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x16a
	.uleb128 0xc
	.string	"i"
	.byte	0x1
	.byte	0x10
	.long	0x57
	.byte	0
	.uleb128 0xd
	.string	"a"
	.byte	0x1
	.byte	0x4
	.long	0x57
	.uleb128 0x9
	.byte	0x3
	.quad	a
	.uleb128 0xd
	.string	"b"
	.byte	0x1
	.byte	0x4
	.long	0x57
	.uleb128 0x9
	.byte	0x3
	.quad	b
	.uleb128 0xd
	.string	"p"
	.byte	0x1
	.byte	0x4
	.long	0x1a3
	.uleb128 0x9
	.byte	0x3
	.quad	p
	.uleb128 0xe
	.byte	0x8
	.long	0x57
	.uleb128 0xf
	.long	.LASF20
	.byte	0x1
	.byte	0x5
	.long	0xfd
	.uleb128 0x9
	.byte	0x3
	.quad	jump_destination
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x13
	.byte	0x1
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0
	.quad	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF13:
	.string	"__val"
.LASF20:
	.string	"jump_destination"
.LASF15:
	.string	"__mask_was_saved"
.LASF1:
	.string	"unsigned char"
.LASF14:
	.string	"__jmpbuf"
.LASF12:
	.string	"__jmp_buf"
.LASF2:
	.string	"short unsigned int"
.LASF17:
	.string	"jmp_buf"
.LASF0:
	.string	"long unsigned int"
.LASF19:
	.string	"main"
.LASF18:
	.string	"exception_handler"
.LASF24:
	.string	"__jmp_buf_tag"
.LASF21:
	.string	"GNU C11 5.4.0 20160609 -mtune=generic -march=x86-64 -g -fstack-protector-strong"
.LASF3:
	.string	"unsigned int"
.LASF10:
	.string	"long long unsigned int"
.LASF23:
	.string	"/home/udit01/assign/os/operating-system-design"
.LASF16:
	.string	"__saved_mask"
.LASF7:
	.string	"sizetype"
.LASF9:
	.string	"long long int"
.LASF8:
	.string	"char"
.LASF5:
	.string	"short int"
.LASF11:
	.string	"__sigset_t"
.LASF6:
	.string	"long int"
.LASF4:
	.string	"signed char"
.LASF22:
	.string	"softISR.c"
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.11) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
