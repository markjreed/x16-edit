;******************************************************************************
;Copyright 2020-2023, Stefan Jakobsson.
;
;This file is part of X16 Edit.
;
;X16 Edit is free software: you can redistribute it and/or modify
;it under the terms of the GNU General Public License as published by
;the Free Software Foundation, either version 3 of the License, or
;(at your option) any later version.
;
;X16 Edit is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with X16 Edit.  If not, see <https://www.gnu.org/licenses/>.
;******************************************************************************


;******************************************************************************
;Function name.......: irq_init
;Purpose.............: Initializes the custom interrupt handler
;Input...............: None
;Returns.............: Nothing
;Error returns.......: None
.if target_mem=target_ram
.proc irq_init
    ;Backup default interrupt handler
    lda $0314
    sta irq_default_handler
    lda $0315
    sta irq_default_handler+1

    ;Set vectors to our custom handler
    sei
    
    stz irq_flag
    lda #<irq_handler
    sta $0314
    lda #>irq_handler
    sta $0315

    cli
    rts
.endproc
.endif

.if target_mem=target_rom
.proc irq_init
    ;Backup default interrupt handler
    lda $0314
    sta irq_default_handler
    lda $0315
    sta irq_default_handler+1

    ;Set vectors to our custom handler
    sei
    
    lda #<(bridge_code+bridge_irq-bridge_kernal)
    sta $0314
    lda #>(bridge_code+bridge_irq-bridge_kernal)
    sta $0315

    cli
    rts
.endproc
.endif

;******************************************************************************
;Function name.......: irq_restore
;Purpose.............: Restores original IRQ handler
;Input...............: None
;Returns.............: Nothing
;Error returns.......: None
.proc irq_restore
    sei
    lda irq_default_handler
    sta $0314
    lda irq_default_handler+1
    sta $0315
    cli
    rts
.endproc

;******************************************************************************
;Function name.......: irq_handler
;Purpose.............: Custom interrupt handler
;Input...............: None
;Returns.............: Nothing
;Error returns.......: None
.proc irq_handler
    ;Verify vblank interrupt
    lda VERA_ISR
    and #$01
    beq exit

    ;Set IRQ flag
    lda #1
    sta irq_flag
    
exit:
    .if (::target_mem=target_ram)
        jmp (irq_default_handler)
    .endif

    .if (::target_mem=target_rom)
        rts
    .endif
.endproc

.segment "VARS"
    irq_default_handler:    .res 2              ;2 bytes
    irq_flag:               .res 1
.CODE