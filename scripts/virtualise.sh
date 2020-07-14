# ---------------------------------------------------------------------------
# virtualise.sh: made to run virtual machines, made by the larrabyte himself.
# ---------------------------------------------------------------------------

if [[ $1 == "qemu" ]]; then
    qemu-system-x86_64 -no-reboot -no-shutdown -serial stdio -display sdl -M q35,accel=whpx:kvm:tcg -cdrom build/freelsd.iso
elif [[ $1 == "bochs" ]]; then
    bochs -q -f scripts/bochsrc.bxrc
fi
