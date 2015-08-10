<?php

echo "float values[50] = {\n";
for ($i=0; $i<50; $i++) {
    echo sin(2*pi()*($i/50)).", ";
    if (($i+1)%10 == 0) echo "\n";
}
echo "};\n";
