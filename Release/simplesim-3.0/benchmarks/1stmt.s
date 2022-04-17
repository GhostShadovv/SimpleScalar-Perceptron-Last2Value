gcc2_compiled.:
___gnu_compiled_c:
.text
	.align 4
	.proc	0110
_label_rtx:
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	st %i0,[%fp+68]
	ld [%fp+68],%o0
	ldub [%o0+12],%o1
	and %o1,0xff,%o0
	cmp %o0,40
	be L2
	nop
	call _abort,0
	nop
L2:
	ld [%fp+68],%o0
	ld [%o0+64],%o1
	cmp %o1,0
	be L3
	nop
	ld [%fp+68],%o0
	ld [%o0+64],%i0
	b L1
	nop
L3:
	call _gen_label_rtx,0
	nop
	ld [%fp+68],%o1
	st %o0,[%o1+64]
	mov %o0,%i0
	b L1
	nop
L1:
	ret
	restore
	.align 4
	.global _emit_jump
	.proc	020
_emit_jump:
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	st %i0,[%fp+68]
	call _do_pending_stack_adjust,0
	nop
	ld [%fp+68],%o0
	call _gen_jump,0
	nop
	mov %o0,%o1
	mov %o1,%o0
	call _emit_jump_insn,0
	nop
	call _emit_barrier,0
	nop
L4:
	ret
	restore
	.align 4
	.global _expand_label
	.proc	020
_expand_label:
	!#PROLOGUE# 0
	save %sp,-120,%sp
	!#PROLOGUE# 1
	st %i0,[%fp+68]
	call _do_pending_stack_adjust,0
	nop
	ld [%fp+68],%o0
	call _label_rtx,0
	nop
	mov %o0,%o1
	mov %o1,%o0
	call _emit_label,0
	nop
	sethi %hi(_stack_block_stack),%o0
	ld [%o0+%lo(_stack_block_stack)],%o1
	cmp %o1,0
	be L6
	nop
	mov 8,%o0
	call _oballoc,0
	nop
	st %o0,[%fp-20]
	ld [%fp-20],%o0
	sethi %hi(_stack_block_stack),%o2
	ld [%o2+%lo(_stack_block_stack)],%o1
	ld [%o1+36],%o2
	st %o2,[%o0]
	sethi %hi(_stack_block_stack),%o1
	ld [%o1+%lo(_stack_block_stack)],%o0
	ld [%fp-20],%o1
	st %o1,[%o0+36]
	ld [%fp-20],%o0
	ld [%fp+68],%o1
	st %o1,[%o0+4]
L6:
L5:
	ret
	restore
	.align 4
	.global _expand_goto
	.proc	020
_expand_goto:
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	st %i0,[%fp+68]
	ld [%fp+68],%o0
	call _label_rtx,0
	nop
	mov %o0,%o1
	ld [%fp+68],%o0
	mov 0,%o2
	call _expand_goto_internal,0
	nop
L7:
	ret
	restore
	.align 8
LC0:
	.ascii "jump to `%s' invalidly jumps into binding contour\0"
	.align 4
	.proc	020
_expand_goto_internal:
	!#PROLOGUE# 0
	save %sp,-120,%sp
	!#PROLOGUE# 1
	st %i0,[%fp+68]
	st %i1,[%fp+72]
	st %i2,[%fp+76]
	st %g0,[%fp-24]
	ld [%fp+72],%o0
	lduh [%o0],%o1
	sll %o1,16,%o2
	srl %o2,16,%o0
	cmp %o0,17
	be L9
	nop
	call _abort,0
	nop
L9:
	ld [%fp+72],%o0
	ld [%o0+8],%o1
	cmp %o1,0
	be L10
	nop
	sethi %hi(_block_stack),%o0
	ld [%o0+%lo(_block_stack)],%o1
	st %o1,[%fp-20]
L11:
	ld [%fp-20],%o0
	cmp %o0,0
	be L12
	nop
	ld [%fp-20],%o0
	ld [%o0+20],%o1
	ld [%fp+72],%o0
	ld [%o1+4],%o1
	ld [%o0+4],%o0
	cmp %o1,%o0
	bge L14
	nop
	b L12
	nop
L14:
	ld [%fp-20],%o0
	ld [%o0+16],%o1
	cmp %o1,0
	be L15
	nop
	ld [%fp-20],%o0
	ld [%o0+16],%o1
	st %o1,[%fp-24]
L15:
	ld [%fp-20],%o0
	ld [%o0+28],%o1
	cmp %o1,0
	be L16
	nop
	ld [%fp-20],%o1
	ld [%o1+28],%o0
	mov 0,%o1
	call _expand_cleanups,0
	nop
L16:
L13:
	ld [%fp-20],%o0
	ld [%o0+4],%o1
	st %o1,[%fp-20]
	b L11
	nop
L12:
	ld [%fp-24],%o0
	cmp %o0,0
	be L17
	nop
	sethi %hi(_stack_pointer_rtx),%o1
	ld [%o1+%lo(_stack_pointer_rtx)],%o0
	ld [%fp-24],%o1
	call _emit_move_insn,0
	nop
L17:
	ld [%fp+68],%o0
	cmp %o0,0
	be L18
	nop
	ld [%fp+68],%o0
	ld [%o0+12],%o1
	sethi %hi(524288),%o2
	and %o1,%o2,%o0
	cmp %o0,0
	be L18
	nop
	ld [%fp+68],%o0
	ld [%o0+36],%o1
	sethi %hi(LC0),%o2
	or %o2,%lo(LC0),%o0
	ld [%o1+20],%o1
	call _error,0
	nop
L18:
	b L19
	nop
L10:
	ld [%fp+68],%o0
	ld [%fp+72],%o1
	ld [%fp+76],%o2
	call _expand_fixup,0
	nop
	cmp %o0,0
	bne L20
	nop
	ld [%fp+68],%o0
	cmp %o0,0
	be L21
	nop
	ld [%fp+68],%o0
	ld [%o0+12],%o1
	sethi %hi(16384),%o2
	or %o1,%o2,%o1
	st %o1,[%o0+12]
L21:
L20:
L19:
	ld [%fp+72],%o0
	call _emit_jump,0
	nop
L8:
	ret
	restore
