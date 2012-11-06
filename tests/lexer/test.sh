for i in *.in
do
    ./lexer.out $i 2> $i.out
done
