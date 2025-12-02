#include <stdio.h>
#include <unistd.h>
#include "./whiteList.h"
#include "./ipset.h"
#include "./logIp.h"

int main() {
	// whiteList.c 실행 -> 파일 생성-> ipset까지 넘어가 ip table 설정 완료
	checkFile();
	//eventScore에서 이벤트종류,ip파싱,점수매칭
	//logIp.c로 화이트리스트,국가 비교 ->if문으로 1이면 riskEngine.c넘어감
	return 1;
}
