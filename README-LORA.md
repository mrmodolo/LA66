# Dragino LA66 — Guia de Uso

Placa: **Dragino LA66 USB** (SoC ASR6601 / Tremo, rádio SX126x integrado)  
Firmware: **DRAGINO-LRWAN-AT** com suporte P2P adicionado (`AT+TEST`)  
Porta serial: `/dev/ttyUSB0` | Baud: **9600** | `\r\n` como terminador

---

## 1. Gravando o Firmware

### Pré-requisitos

```bash
# Ativar o venv com pyserial
source /srv/Playground/LA66-1.3/.venv/bin/activate
```

### Compilar

```bash
TREMO_SDK_PATH=/srv/Playground/LA66-1.3 \
  make -C Projects/Applications/DRAGINO-LRWAN-AT TREMO_SDK_PATH=/srv/Playground/LA66-1.3
# Binário em: Projects/Applications/DRAGINO-LRWAN-AT/Make_out/DRAGINO-LRWAN-AT.bin
```

### Procedimento de gravação

1. **Instalar JP1** (jumper de ISP) na placa
2. **Reconectar o USB** (ou pressionar reset) — a placa entra em modo de gravação
3. **Gravar nos dois endereços** (obrigatório — o bootloader lê o vetor de reset em `0x08000000` mas o código reside em `0x0800D000`):

```bash
python build/scripts/tremo_loader.py -p /dev/ttyUSB0 -b 921600 flash \
  0x08000000 Projects/Applications/DRAGINO-LRWAN-AT/Make_out/DRAGINO-LRWAN-AT.bin \
  0x0800D000 Projects/Applications/DRAGINO-LRWAN-AT/Make_out/DRAGINO-LRWAN-AT.bin
```

4. **Remover JP1** → reconectar USB (ou pressionar reset) → placa inicia normalmente

> **Por que dois endereços?** O linker coloca o código em `ORIGIN = 0x0800D000` (ver `cfg/gcc.ld`). O ROM bootloader lê o vetor de reset sempre de `0x08000000`. Gravar apenas um endereço faz a placa não iniciar.

---

## 2. Interface AT

Abrir o terminal serial a **9600 baud**:

```bash
minicom -D /dev/ttyUSB0 -b 9600
# ou
screen /dev/ttyUSB0 9600
```

Verificar comunicação:

```
AT
# resposta: OK
```

Listar todos os comandos disponíveis:

```
AT?
```

---

## 3. Modo LoRaWAN (OTAA)

### Configuração inicial

```
# Verificar EUI do dispositivo (gerado automaticamente)
AT+DEUI=?

# Configurar credenciais OTAA (obter no servidor LoRaWAN)
AT+APPEUI=0000000000000000
AT+APPKEY=00000000000000000000000000000000

# Confirmar modo OTAA (1 = OTAA, 0 = ABP)
AT+NJM=?       # deve retornar 1

# Confirmar região (compilado como US915)
AT+VER=?
```

### Fazer Join

```
AT+JOIN
```

A placa tenta o join OTAA automaticamente. Aguardar a confirmação da rede (LED azul pisca no downlink). Verificar status:

```
AT+NJS=?
# 0 = não conectado
# 1 = conectado (joined)
```

### Enviar dados (uplink)

```
AT+SEND=<confirm>,<fport>,<length>,<payload_ASCII>
```

| Parâmetro | Valores | Descrição |
|-----------|---------|-----------|
| `confirm` | `0` / `1` | 0 = unconfirmed, 1 = confirmed |
| `fport`   | `1`–`223` | FPort LoRaWAN |
| `length`  | nº de bytes | Tamanho exato do payload |
| `payload` | texto ASCII | Dados a enviar |

Exemplos:

```
AT+SEND=0,1,5,Hello        # envia "Hello" não confirmado no fport 1
AT+SEND=1,2,3,ABC          # envia "ABC" confirmado no fport 2
```

Envio em hex (binário):

```
AT+SENDB=<confirm>,<fport>,<length>,<payload_HEX>
AT+SENDB=0,1,4,AABBCCDD
```

### Receber dados (downlink)

O último downlink recebido fica em buffer. Para ler:

```
AT+RECV=?        # formato: <fport>:<payload_ASCII>
AT+RECVB=?       # formato: <fport>:<payload_HEX>
```

O LED vermelho + azul pisca 200 ms a cada downlink recebido.

### Configurações úteis

```
AT+DR=?          # Data Rate atual (0–7, DR0=SF10/125kHz em US915)
AT+DR=2          # Forçar DR2 (ADR desativado)
AT+ADR=1         # Ativar Adaptive Data Rate
AT+CFM=0         # Mensagens não confirmadas (default)
AT+CFM=1         # Mensagens confirmadas (aguarda ACK)
AT+PORT=1        # Definir FPort padrão
AT+RSSI=?        # RSSI do último downlink
AT+SNR=?         # SNR do último downlink
```

### Restaurar configurações de fábrica

```
AT+FDR
```

---

## 4. Modo P2P (LoRa direto, sem gateway)

O modo P2P usa o rádio diretamente, sem stack LoRaWAN. Útil para comunicar com outra placa LoRa (Arduino, LILYGO, etc.) sem infraestrutura.

> **Nota:** Durante P2P o stack LoRaWAN é pausado (timers suspensos). Após os comandos `TXLRPKT`/`RXLRPKT` o stack retoma automaticamente.

### Passo 1 — Configurar o rádio

```
AT+TEST=RFCFG,<freq_MHz>,<SF>,<BW_kHz>,<TX_preamble>,<RX_preamble>,<power_dBm>[,<sync_word>]
```

| Parâmetro | Faixa | Descrição |
|-----------|-------|-----------|
| `freq_MHz` | ex: `915.125` | Frequência em MHz (suporta decimal) |
| `SF` | `7`–`12` | Spreading Factor |
| `BW_kHz` | `125` / `250` / `500` | Largura de banda |
| `TX_preamble` | `8` (recomendado) | Comprimento do preâmbulo TX |
| `RX_preamble` | `8` (recomendado) | Comprimento do preâmbulo RX |
| `power_dBm` | `2`–`22` | Potência de transmissão |
| `sync_word` | `0` (padrão) / `1` | `0` = privado `0x1424`, `1` = público `0x3444` (LoRaWAN) |

O parâmetro `sync_word` é opcional. O padrão (`0`) usa o sync word privado `0x1424`, compatível com a Arduino LoRa library e com a maioria dos módulos não-LoRaWAN.

Exemplo (compatível com Arduino LoRa library padrão):

```
AT+TEST=RFCFG,915.125,7,125,8,8,14
# resposta: +TEST: RFCFG F:915125000 Hz, SF7, BW125kHz, TX_PRE:8, RX_PRE:8, PWR:14, SW:PRIVATE(0x1424)
#           OK
```

Exemplo com sync word público (para comunicar com LoRaWAN ou dispositivos que usam `0x3444`):

```
AT+TEST=RFCFG,915.125,7,125,8,8,14,1
# resposta: +TEST: RFCFG F:915125000 Hz, SF7, BW125kHz, TX_PRE:8, RX_PRE:8, PWR:14, SW:PUBLIC(0x3444)
#           OK
```

> A configuração persiste até a próxima chamada de `RFCFG` ou reset.  
> Padrões após reset: 915.125 MHz, SF10, BW 125 kHz, preamble 8/8, 14 dBm, sync word privado.

### Passo 2 — Transmitir um pacote

```
AT+TEST=TXLRPKT,<payload_HEX>
```

O payload deve ser uma string hexadecimal (2 chars por byte, sem espaços, sem aspas):

```
AT+TEST=TXLRPKT,48656C6C6F     # "Hello" em hex
AT+TEST=TXLRPKT,AABBCCDD0102

# resposta em caso de sucesso:
# +TEST: TX DONE
# OK

# resposta em caso de timeout (3 s):
# +TEST: TX TIMEOUT
# OK
```

O LED verde pisca durante a transmissão.

Converter texto para hex no terminal Linux:

```bash
echo -n "HelloLA66" | xxd -p   # → 48656c6c6f4c413636
```

### Passo 3 — Receber um pacote

```
AT+TEST=RXLRPKT
```

A placa fica em modo de recepção contínua por **5 segundos** aguardando um pacote. Use os mesmos parâmetros configurados em `RFCFG` da outra placa.

```
# pacote recebido:
AT+TEST=RXLRPKT
+TEST: RX "4F6CC3A1", RSSI:-41, SNR:12
OK

# sem pacote em 5 s:
AT+TEST=RXLRPKT
+TEST: RX TIMEOUT
OK

# pacote com erro de CRC:
AT+TEST=RXLRPKT
+TEST: RX CRC ERROR
OK
```

O payload é retornado em hexadecimal. Para decodificar:

```bash
echo "4F6CC3A1" | xxd -r -p   # → Olá
```

---

## 5. Comunicação entre LA66 e LILYGO (Arduino LoRa)

A biblioteca Arduino LoRa usa **sync word privado** (`0x12`), enquanto o stack LoRaWAN usa o público (`0x34`). O firmware AT+TEST já trata essa diferença automaticamente — `Radio.SetPublicNetwork(false)` é chamado internamente nos comandos TXLRPKT e RXLRPKT.

### Configuração nos dois lados

| Parâmetro | LA66 (`AT+TEST=RFCFG`) | Arduino / LILYGO |
|-----------|------------------------|------------------|
| Frequência | `915.125` MHz | `LoRa.begin(915125000)` |
| SF | `7` | `LoRa.setSpreadingFactor(7)` |
| BW | `125` kHz | `LoRa.setSignalBandwidth(125E3)` |
| CR | 4/5 (fixo) | `LoRa.setCodingRate4(5)` |
| Preamble | `8` | `LoRa.setPreambleLength(8)` |
| Sync word | privado (automático) | `LoRa.setSyncWord(0x12)` |

### Exemplo de sessão completa

**Terminal LA66** (ttyUSB0, 9600 baud):

```
# 1. Configurar P2P
AT+TEST=RFCFG,915.125,7,125,8,8,14

# 2. Receber pacote da LILYGO
AT+TEST=RXLRPKT
+TEST: RX "4F6CC3A1", RSSI:-41, SNR:12
OK

# 3. Enviar resposta para a LILYGO
AT+TEST=TXLRPKT,48656C6C6F4C413636
+TEST: TX DONE
OK
```

---

## 6. Transição entre modos

### P2P → LoRaWAN

Após os comandos `TXLRPKT`/`RXLRPKT`, o firmware restaura automaticamente:
- Sync word público (`0x3444`)
- Timers do LoRaWAN (RTC reativado)

Para forçar um novo join depois de usar P2P:

```
AT+JOIN
```

### LoRaWAN → P2P

Basta chamar `AT+TEST=RFCFG,...` seguido de `TXLRPKT` ou `RXLRPKT`. O firmware cuida da suspensão do stack automaticamente.

---

## 7. LEDs de status

| LED | Cor | Evento |
|-----|-----|--------|
| RED + BLUE | 200 ms | Downlink LoRaWAN recebido |
| GREEN | 200 ms | Uplink LoRaWAN transmitido |

---

## 8. Referência rápida

```
AT              → testa comunicação
AT+VER=?        → versão do firmware
AT+DEUI=?       → Device EUI (para registro no servidor)
AT+NJM=1        → modo OTAA
AT+NJM=0        → modo ABP
AT+JOIN         → iniciar join OTAA
AT+NJS=?        → status do join (0/1)
AT+SEND=0,1,N,payload   → uplink ASCII não confirmado
AT+SENDB=0,1,N,HEXHEX   → uplink hex não confirmado
AT+RECV=?       → ler último downlink (ASCII)
AT+RECVB=?      → ler último downlink (hex)
AT+RSSI=?       → RSSI do último downlink
AT+SNR=?        → SNR do último downlink
AT+ADR=1        → ativar ADR
AT+DR=?         → data rate atual
AT+FDR          → reset de fábrica
AT+TEST=RFCFG,freq,SF,BW,txPre,rxPre,pwr[,sw]  → configurar P2P (sw: 0=privado, 1=público)
AT+TEST=TXLRPKT,HEX     → transmitir pacote P2P
AT+TEST=RXLRPKT          → receber pacote P2P (5 s)
AZ              → reset do MCU
```
