# 📘 ptmt
authlog파일 기반 ip대역 추출 후 자동차단 프로그램
# 🚀 Features

## whiteList.c 

- 파일 권한 확인 후 생성
- 잘못된 IP 또는 추가 실패 시 오류 출력
- root 권한 기반 안전한 시스템 명령 실행
- 간단한 C 기반 CLI 프로그램

## ipset.c
- `whiteList.conf` 파일에서 IP를 자동으로 읽어 처리
- 불필요한 공백 및 개행문자 자동 제거(trim)
- ipset 명령(`ipset add whitelist <IP>`) 자동 실행

## ptmt-collector (auth.log 실시간 감시 및 로그 수집기)
- inotify(IN_MODIFY) 기반 실시간 auth.log append 감지
- 새로 추가된 라인만 추출하여 별도 파일로 저장
- Makefile 기반 자동 빌드 및 /usr/local/bin/ptmt-collector 명령으로 실행
---

# 🛠️ First-Time Setup (첫 사용 시 필수 설정)

프로그램을 실행하기 위해서는 Linux 커널에 **화이트리스트용 ipset 세트**를 만들어야 함  
이 작업은 **최초 1회만 실행**

## 🔧 1. ipset 세트 생성

```bash
sudo ipset create whitelist hash:ip
```
## 🔧 2. Run
```bash
gcc ipset.c whiteList.c -o whiteList
sudo ./whiteList
```
## 🔧 3. ptmt-collector Setup & Usage
```bash
sudo apt install make
make run
sudo ptmt-collector
```


