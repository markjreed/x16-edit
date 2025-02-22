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
;Function name.......: scancode_init
;Purpose.............: Initializes keyboard scan code handler
;Input...............: None
;Returns.............: Nothing
;Error returns.......: None
.if target_mem=target_ram
.proc scancode_init
    lda $032e
    sta scancode_default_handler
    lda $032f
    sta scancode_default_handler+1
    
    stz scancode_modifiers

    sei
    lda #<scancode_handler
    sta $032e
    lda #>scancode_handler
    sta $032f
    cli
    
    rts
.endproc
.endif

.if target_mem=target_rom
.proc scancode_init
    lda $032e
    sta scancode_default_handler
    lda $032f
    sta scancode_default_handler+1

    stz scancode_modifiers

    sei
    lda #<(bridge_code+bridge_scancode-bridge_kernal)
    sta $032e
    lda #>(bridge_code+bridge_scancode-bridge_kernal)
    sta $032f
    cli
    
    rts
.endproc
.endif


;******************************************************************************
;Function name.......: scancode_restore
;Purpose.............: Restores scan code handler to default value
;Input...............: None
;Returns.............: Nothing
;Error returns.......: None
.proc scancode_restore
    sei
    lda scancode_default_handler
    sta $032e
    lda scancode_default_handler+1
    sta $032f
    cli
    
    rts
.endproc

;******************************************************************************
;Function name.......: scancode_handler
;Purpose.............: Custom keycode handler
;Input...............: Input set by Kernal, A=key code
;Returns:............: Modifier key status is stored in variable
;                      scancode_modifier, each bit representing one key
;                      (1=down, 0=up) as follows: Bit 0=Shift, bit 1=Alt,
;                      bit 2=Ctrl, bit 3=Windows key, bit 4=Caps lock
;                      Ctrl key presses are consumed and not relayed
;                      to the Kernal.
;Error returns:......: None
.proc scancode_handler
    ;Save input so we can restore value later
    pha

    ;Check keycode match
    and #%01111111      ; Filter out up/down bit
    ldy #9

loop:
    cmp keycodes-1,y
    beq match
    dey
    bne loop

nomatch:
    pla                 ; Retore keycode from stack and exit
    rts

match:
    lda bitmask-1,y
    plx                 ; Get keycode from stack, temporarily stored in X
    bmi keyup           ; Bit 7=1 is keyup

keydown:
    ora scancode_modifiers
    bra :+

keyup:
    eor #$ff
    and scancode_modifiers

:   ;Update modifier status
    sta scancode_modifiers
    
    ; Consume Ctrl key events
    lda bitmask-1,y
    and #KBD_MODIFIER_CTRL
    bne consume

    ; Consume Shift + Alt to prevent toggle between upper/lower case
    lda scancode_modifiers
    and #KBD_MODIFIER_SHIFT+KBD_MODIFIER_ALT
    cmp #KBD_MODIFIER_SHIFT+KBD_MODIFIER_ALT
    beq consume

    ; Move keycode from X to A and exit
    txa
    rts

consume:
    lda #0
    rts

keycodes:
    .byt KEYCODE_LSHIFT
    .byt KEYCODE_RSHIFT
    .byt KEYCODE_LALT
    .byt KEYCODE_RALT
    .byt KEYCODE_LCTRL
    .byt KEYCODE_RCTRL
    .byt KEYCODE_LGUI
    .byt KEYCODE_RGUI
    .byt KEYCODE_CAPSLOCK

bitmask:
    .byt KBD_MODIFIER_SHIFT, KBD_MODIFIER_SHIFT, KBD_MODIFIER_ALT
    .byt KBD_MODIFIER_RALT, KBD_MODIFIER_CTRL, KBD_MODIFIER_CTRL
    .byt KBD_MODIFIER_WIN, KBD_MODIFIER_WIN, KBD_MODIFIER_CAPS

.endproc

.segment "VARS"
    scancode_default_handler: .res 2
    scancode_modifiers: .res 1
.CODE

;Modifier key bitmask
KBD_MODIFIER_SHIFT = 1 ; C64:  Shift
KBD_MODIFIER_ALT   = 2 ; C64:  Commodore
KBD_MODIFIER_CTRL  = 4 ; C64:  Ctrl
KBD_MODIFIER_WIN   = 8 ; C128: Alt
KBD_MODIFIER_CAPS  = 16; C128: Caps
KBD_MODIFIER_RALT  = 32; Right Alt key

;Modifier key codes
KEYCODE_LSHIFT      = $2C
KEYCODE_RSHIFT      = $39
KEYCODE_LCTRL       = $3A
KEYCODE_LGUI        = $3B
KEYCODE_LALT        = $3C
KEYCODE_RALT        = $3E
KEYCODE_RGUI        = $3F
KEYCODE_RCTRL       = $40
KEYCODE_CAPSLOCK    = $1E