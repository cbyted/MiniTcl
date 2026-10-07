set total 0
for {set i 1} {$i <= 5} {incr i} {
    set total [expr {$total + $i}]
}
puts "Total: $total"

