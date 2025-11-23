for i in $(seq -w 0 99); do
    echo "Executando COMPILADOR pass$i"
    ./etapa5 < meus_testes/test/pass/pass$i > meus_testes/test/pass/pass$i.iloc
done