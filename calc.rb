OUTFILEROOT=File.basename(__FILE__,".rb")

# We're going to generate a lookup table to map ADC values
# (0-1023) to phase-increments. Each octave will be split
# into 128 steps, giving a range of 8 octaves.

# PWM rate is 250kHz, we'd like an integer-ratio of this
# for our sample rate. Also, bear in mind that we are
# generating 2 oscillators, so the interrupt will actually
# be running at twice this rate...
SRATE=50000
PHASECOUNTERBITS = 16
INTBITS	         = 10
FRACBITS         = PHASECOUNTERBITS-INTBITS

# Now then. Although we are declaring that the WTSIZE
# is this, the actual size is going to be half of
# this because that's what Wolfgang Palm did: realise
# that, if you keep waves symmetrical, you only
# need half of the space…
WTSIZE           = (2.0**INTBITS).to_i

# root frequency is that of A0 (according to Dodge and Jerse pg.37)
ROOT             = 27.50

DACBITS          = 10
DACRANGE         = (2.0**DACBITS).to_i
# 128 steps per octave means that the 10 bit ADC covers 8 octaves
OCTSTEPS=128
OCTS=(DACRANGE/OCTSTEPS)

File.open("#{OUTFILEROOT}.h",'w') do |f|
  f.puts "#include \"arduino.h\"\n"
  f.puts "#define SRATE    (#{SRATE}L)"
  f.puts "#define WTSIZE   (#{WTSIZE/2}L)"
  f.puts "#define FRACBITS (#{FRACBITS}L)"
  f.puts "#define HALF     (0x#{(1 << (FRACBITS-1)).to_s(16)})"
  f.puts "#define DACRANGE (#{DACRANGE}L)"
  f.puts "extern const uint16_t octaveLookup[DACRANGE];"
  f.puts "extern const uint8_t  sine[WTSIZE];"
  f.puts "extern const uint8_t  ramp[WTSIZE];"
  f.puts "extern const uint8_t  sq[WTSIZE];"
  f.puts "extern const uint8_t  triangle[WTSIZE];"
  f.puts
end

# Let's calculate a single octave's worth of increments,
# subsequent octaves are simply power-of-2 multiples of those
# base octave values

File.open("#{OUTFILEROOT}.ino",'w') do |f|
  f.puts "#include \"#{OUTFILEROOT}.h\""
  f.puts "const uint16_t octaveLookup[DACRANGE] PROGMEM = {"

  (0...DACRANGE).each do |n|
    freq = ROOT * (2.0**(n.to_f/OCTSTEPS))
    # Dodge & Jerse (pg.67) say that the sample increment is WTSIZE*(fo/fs)
    si = (WTSIZE * freq)/SRATE
    sifixed = ((WTSIZE << FRACBITS)* freq)/SRATE
    f.puts "  0x#{sifixed.to_i.to_s(16)}, // #{freq} [#{si}]"
  end
  f.puts "};\n"

  # generate a sine table.
  TWO_PI = 2 * Math::PI
  LINELENGTH = 16
  f.puts "const uint8_t sine[WTSIZE] PROGMEM = {"
  f.print "  "
  (0...WTSIZE/2).each do |n|
    # although we only need half of the wave in the table,
    # we still treat the wave as if it were whole.
    v = Math::sin(TWO_PI * n.to_f/WTSIZE.to_f)
    f.print "0x#{(((v*127)+128)/2).to_i.to_s(16)}, "
    if ((n % LINELENGTH) == (LINELENGTH-1))
      f.print "\n  "
    end
  end
  f.puts "};"

  f.puts "const uint8_t ramp[WTSIZE] PROGMEM = {"
  f.print "  "
  (0...WTSIZE/2).each do |n|
    f.print "0x#{((128+(n/4))/2).to_s(16)}, "
    if ((n % LINELENGTH) == (LINELENGTH-1))
      f.print "\n  "
    end
  end
  f.puts "};"

  f.puts "const uint8_t sq[WTSIZE] PROGMEM = {"
  f.print "  "
  (0...WTSIZE/2).each do |n|
    f.print "0x7f, "
    if ((n % LINELENGTH) == (LINELENGTH-1))
      f.print "\n  "
    end
  end
  f.puts "};"

  f.puts "const uint8_t triangle[WTSIZE] PROGMEM = {"
  f.print "  "
  (0...WTSIZE/2).each do |n|
    if (n < WTSIZE/4)
      f.print "0x#{((128+n/2)/2).to_s(16)}, "
    else
      f.print "0x#{((383-n/2)/2).to_s(16)}, "
    end
    if ((n % LINELENGTH) == (LINELENGTH-1))
      f.print "\n  "
    end
  end
  f.puts "};"
end
