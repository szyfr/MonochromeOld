;*/
;* @Date:		5/4/21
;* @Author:		Szyfr
;*/


;;=  Creates a memory heap and cleans it
;;=		Input:		RCX = size
;;=		Output:		RAX = &memoryAddress
ALM_memorySize		equ		rcx
ALM_memoryAddress	equ		rax
ALM_memorySizeAlt	equ		rdx
ALM_memoryPointer	equ		rcx
allocate_memory:
	sub		rsp,	28h

	push	rdx

.allocate:
	push	ALM_memorySize
	call	MemAlloc
.clean:
	pop		ALM_memorySizeAlt
	mov		ALM_memoryPointer,		ALM_memoryAddress
	call	clean_memory
.exit:
	pop		rdx
	add		rsp,	28h
	ret


;;=  Zeros out all of the memory at address
;;=		Input:		RCX = &memoryLocation
;;=					RDX = size
;;=		Output:		N/A
CLM_memoryLocation	equ		rcx
CLM_heapSize		equ		rdx
CLM_counter			equ		rbx
clean_memory:
	sub		rsp,	28h

	push	rbx
	xor		CLM_counter,CLM_counter

.loop:
	mov		byte[CLM_memoryLocation],$0
	inc		CLM_memoryLocation
	inc		CLM_counter

	cmp		CLM_counter,CLM_heapSize
	jnz		.loop

.exit:
	pop		rbx
	add		rsp,	28h
	ret


;;=  Loads contents of a file and stores a pointer to heap
;;=	 Also stores file length at the start of the heap
;;=		Input:		RCX = &filename
;;=					RDX = outputFormat
;;=						0:Regular	1:Double-sized and in later half
;;=		Output:		RAX = &data
;;? Look back at this in a little to see if this is still good.
LDF_output			equ		rax
LDF_oldPtr			equ		rax
LDF_filename		equ		rcx
LDF_outputFormat	equ		rdx

LDF_generalMemory	equ		rdx
LDF_firstAllocSize	equ		rdx
LDF_secondAllocSize	equ		rcx

LDF_fileDataPtr		equ		rdx+60

LDF_byteCounter		equ		rcx
LDF_oldPtrL			equ		r8
LDF_newPtrL			equ		r9
LDF_copiedDword		equ		r10d
load_file_data:
	sub		rsp,	28h

	push	LDF_filename

.loading_file:
	mov		LDF_firstAllocSize,		[general_memory]
	call	LoadFileData

.check_format:
	pop		LDF_filename
	cmp		LDF_filename,			0
	je		.exit

.allocate_new_memory:
	mov		LDF_generalMemory,		[general_memory]
	mov		[LDF_fileDataPtr],		LDF_output

	mov		LDF_secondAllocSize,	[LDF_generalMemory]
	imul	LDF_secondAllocSize,	2
	call	MemAlloc

.copy_init:
	mov		LDF_generalMemory,		[general_memory]
	mov		LDF_byteCounter,		[LDF_generalMemory]
	mov		LDF_oldPtrL,			[LDF_fileDataPtr]
	mov		LDF_newPtrL,			LDF_output
	add		LDF_newPtrL,			LDF_byteCounter

.copy_loop:
	mov		LDF_copiedDword,		[LDF_oldPtrL]
	mov		[LDF_newPtrL],			LDF_copiedDword
	add		LDF_oldPtrL,			4
	add		LDF_newPtrL,			4
	sub		LDF_byteCounter,		4
	jnz		.copy_loop

.remove_old_pointer:
	mov		rcx,				[LDF_fileDataPtr]
	mov		[LDF_fileDataPtr],	LDF_oldPtr
	call	UnloadFileData

	mov		LDF_generalMemory,	[general_memory]
	mov		rcx,				[LDF_generalMemory]
	mov		LDF_output,			[LDF_fileDataPtr]
	mov		[LDF_output],		rcx

.exit:
	add		rsp,	28h
	ret


;;=  Parses file data and creates a table of contents
;;=		Input:		RCX = &data
;;=					RDX	= dataSize
;;=					R8	= endBuffer
;;=		Output:		N/A
;;?  Move this to a seperate file?
PRS_dataPtr			equ		rcx
PRS_dataSize		equ		rdx

PRS_tablePtr		equ		r10
PRS_contentsPtr		equ		r11
PRS_dataCounter		equ		rcx

PRS_buffer_8		equ		r8b
PRS_buffer_16		equ		r8w
PRS_buffer_32		equ		r8d
PRS_buffer_64		equ		r8

PRS_content_8		equ		al
PRS_content_16		equ		ax
PRS_content_32		equ		eax
PRS_content_64		equ		rax
parse_file_data:
	sub		rsp,	28h

	push	r10
	push	r11

.parse_initialize:
	mov		PRS_tablePtr,		PRS_dataPtr
	mov		PRS_contentsPtr,	PRS_dataPtr

	mov		PRS_dataCounter,	[PRS_tablePtr]
	add		PRS_contentsPtr,	PRS_dataCounter

	mov		[PRS_tablePtr],		PRS_contentsPtr
	add		PRS_tablePtr,		8

.parse_table:
	cmp		PRS_dataSize,		1
	je		.parse_loop_8

	cmp		PRS_dataSize,		2
	je		.parse_loop_16

	cmp		PRS_dataSize,		4
	je		.parse_loop_32

	cmp		PRS_dataSize,		8
	je		.parse_loop_64
	

	jmp		.exit

.parse_loop_8:
	mov		PRS_content_8,		[PRS_contentsPtr]
	cmp		PRS_content_8,		PRS_buffer_8
	je		.parse_loop_8_pointer

	add		PRS_contentsPtr,	PRS_dataSize
	sub		PRS_dataCounter,	PRS_dataSize
	jnz		.parse_loop_8

	jmp		.exit
.parse_loop_8_pointer:
	add		PRS_contentsPtr,	PRS_dataSize
	mov		[PRS_tablePtr],		PRS_contentsPtr
	add		PRS_tablePtr,		8
	sub		PRS_dataCounter,	PRS_dataSize
	jnz		.parse_loop_8

	jmp		.exit
.parse_loop_16:
	mov		PRS_content_16,		[PRS_contentsPtr]
	cmp		PRS_content_16,		PRS_buffer_16
	je		.parse_loop_16_pointer

	add		PRS_contentsPtr,	PRS_dataSize
	sub		PRS_dataCounter,	PRS_dataSize
	jnz		.parse_loop_16

	jmp		.exit
.parse_loop_16_pointer:
	add		PRS_contentsPtr,	PRS_dataSize
	mov		[PRS_tablePtr],		PRS_contentsPtr
	add		PRS_tablePtr,		8
	sub		PRS_dataCounter,	PRS_dataSize
	jnz		.parse_loop_16

	jmp		.exit
.parse_loop_32:
	mov		PRS_content_32,		[PRS_contentsPtr]
	cmp		PRS_content_32,		PRS_buffer_32
	je		.parse_loop_32_pointer

	add		PRS_contentsPtr,	PRS_dataSize
	sub		PRS_dataCounter,	PRS_dataSize
	jnz		.parse_loop_32

	jmp		.exit
.parse_loop_32_pointer:
	add		PRS_contentsPtr,	PRS_dataSize
	mov		[PRS_tablePtr],		PRS_contentsPtr
	add		PRS_tablePtr,		8
	sub		PRS_dataCounter,	PRS_dataSize
	jnz		.parse_loop_32

	jmp		.exit
.parse_loop_64:
	mov		PRS_content_64,		[PRS_contentsPtr]
	cmp		PRS_content_64,		PRS_buffer_64
	je		.parse_loop_64_pointer

	add		PRS_contentsPtr,	PRS_dataSize
	sub		PRS_dataCounter,	PRS_dataSize
	jnz		.parse_loop_64

	jmp		.exit
.parse_loop_64_pointer:
	add		PRS_contentsPtr,	PRS_dataSize
	mov		[PRS_tablePtr],		PRS_contentsPtr
	add		PRS_tablePtr,		8
	sub		PRS_dataCounter,	PRS_dataSize
	jnz		.parse_loop_64

	jmp		.exit

.exit:
	pop		r11
	pop		r10

	add		rsp,28h
	ret