// Additional nullptr check.
cmplwi cr6, r3, 0
beq cr6, jEnd

// Original instructions.
lwz       r11, 0(r3)
mr        r4, r30
lwz       r10, 0x1B4(r11)
mtctr     r10
bctrl

jEnd:

// Jmp after patch.
lis r0, 0x8874
ori r0, r0, 0x53EC
mtctr r0
bctr