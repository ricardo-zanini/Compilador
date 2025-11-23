for i in $(seq -w 0 99); do
    echo "Executando pass$i.iloc"
    python3 ilocsim.py meus_testes/test/pass/pass$i.iloc
done