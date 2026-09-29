set architecture i8086

display/i (($cs * 16) + $pc)

define xi
  x/20i (($cs * 16) + $pc)
end

define sii
  si
  x/10i (($cs * 16) + $pc)
end
