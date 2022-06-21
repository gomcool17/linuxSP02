# linuxSP02
리눅스시스템프로그래밍 과제2

# 1. 개요
 데몬이란 서비스의 요청에 대해 응답하는 오랫동안 실행중인 백그라운드 프로세스다. 백그라운드와 반대 개념인 포그라운드 프로세스도 있는데 포그라운드 프로세는 사용자와의 대화창구인 표준입출력장치를 통해 대화한다. 그러나 백그라운드 프로세스는 적어도 입력장치에 대해 터미널과의 관계를 끊은 모든 프로세스를 의미한다. 즉 사용자에게 무언가를 키보드를 통해 전달받지 않고 스스로 동작하는 프로세스가 바로 백그라운드 프로세스이다. 본과제는 백그라운드 프로세스를 이해해서 직접 만들고 백그라운드 프로세스를 통해 파일의 생성, 삭제, 수정여부를 자동으로 txt파일에 적히게 한다. 또한 파일의 정보들을 통해 파일의 이동(삭제), 삭제등 여러가지 파일을 다루는 법을 익힌다. 또한 alarm system함수를 통해 시스템 콜에 대한 이해도를 높힌다.

----

# 2. 설계
<img width="487" alt="image" src="https://user-images.githubusercontent.com/30142575/174763693-a20decd3-39e7-4e6c-a223-f37c3e41042d.png">
   디몬프로세스를 생성하기 위해서는 명령어들을 실행해주는 실행파일과 별도로 다른 실행파일이 필요하다. main.c에서 자식프로세스를 생성해준 다음 execl - ssu_daemon 을 통해 연결시켜준. ssu_daemon.c에서도 자식프로세스를 실행시켜주면 main.c프로그램을 종료하더라도 백그라운드는 남아있다. 이를통해 log.txt에 실시간 파일 생성여부를 찍어줄수 있다. 실시간 파일 생성, 삭제, 수정은 ssu_daemon.c을 실행할 당시에 전역변수에 존재하던 파일들의 이름과 시간등 정보를 저장해준다. 그다음 모니터링 함수를 계속 호출시키면서 모니터링 함수안에서 다시 실시간 파일개수, 파일의 시간정보등을 받아온다. 그다음 전역변수랑 비교를 하면 생성, 삭제, 수정여부를 알 수 있다. 가장 큰 비교대상은 파일의 개수이다.
 명령어 수행 프로그램은 인자로 받은 정보들을 공백을 기준으로 나누어 저장한다. 첫번째 배열은 무조건 명령어 이름이다. 알맞은 명령어가 없다면 에러처리를 한다. 
 delete명령어는 파일정보가 절대경로, 상대경로 혹은 그냥 파일이름으로 올 수 있다. 그냥 파일이름으로 오면 check디렉토리의 파일로 간주를 한다. Trash, files, info디렉토리가 없다면 생성후 삭제할 파일을 rename으로 files디렉토리에 옮겨준다. 삭제한 파일이름을 가지고 info디렉토리에 새로 파일을 생성해준 다음 파일의 삭제정보와 마지막 수정 정보를 입력해 저장한다.
tree함수는 check디렉토리부터 파일의 정보를 받아와 출력한다. 이때 파일인지 디렉토리인지 반드시 확인을 해야한다. 디렉토리 파일이면 다시 tree함수를 호출하여 또다시 파일의 정보를 받아와 출력한다. 최초 함수호출시 깊이는 0이고 재호출 할때마다 깊이는 +1이된다.
recover함수는 info의 파일들을 탐색하여 중복이 있는지 아닌지 검사를 한다. 중복이 없다면 바로 info에 있는 해당파일은 unlink함수를 통해 삭제를 해주고 files에 있던 해당파일은 rename으로 다시 check디렉토리에 넣어준다. 만일 복구하는 디렉토리에 중복되는 이름이 있다면 n_을 붙여준다 (n은 정수)
fileinfosize함수는 info디렉토리의 파일들 하나하나 사이즈 검사를 해주어 전부 합쳐준다. 만일 2KB인 2048바이트보다 크다면 제일 먼저 삭제된 파일을 삭제해준다. 이때 파일정보는 생성된 시간이므로 시간순으로 정렬을 해준다.

---

# 3. 실행화면
+ tree 명령어
<img width="435" alt="image" src="https://user-images.githubusercontent.com/30142575/174764783-5e562797-4980-4324-bbd7-859bc2c399f2.png">

+ delete 명령어
<img width="486" alt="image" src="https://user-images.githubusercontent.com/30142575/174764798-e7a6a210-37d2-4028-bf8b-57367afe0bce.png">
<img width="304" alt="image" src="https://user-images.githubusercontent.com/30142575/174764825-d124dc57-1396-430c-bcf5-d79e35d70f2c.png">
<img width="338" alt="image" src="https://user-images.githubusercontent.com/30142575/174764842-561d93c6-6840-47f9-9e86-e379c9999b75.png">
<img width="303" alt="image" src="https://user-images.githubusercontent.com/30142575/174764851-cdba550b-e9b1-4594-862a-9a28d26e3565.png">

+ recover 명령어
  + 중복된 이름이 있을 시
<img width="434" alt="image" src="https://user-images.githubusercontent.com/30142575/174765184-22887ba9-2976-42a2-8553-bd891224c458.png">

  + 복구된 파일

<img width="296" alt="image" src="https://user-images.githubusercontent.com/30142575/174765233-126d0152-4772-4cec-8833-9ac15bec48e6.png">

  + 2번을 선택했기 때문에 1_1.c가 복구됨을 확인

<img width="294" alt="image" src="https://user-images.githubusercontent.com/30142575/174765254-5f5bfd62-63aa-485a-984a-5a3cf3aaf7f9.png">

  + 파일의 정보(info)도 지워졌음을 확인

<img width="309" alt="image" src="https://user-images.githubusercontent.com/30142575/174765276-98bd2a6d-5154-4c92-9bb6-19034e720c08.png">

  + 중복된 이름 복구

<img width="159" alt="image" src="https://user-images.githubusercontent.com/30142575/174765298-4b6e5c52-6de8-4f46-bb01-c97ce350db32.png">

  + 1_가 붙음을 확인

<img width="282" alt="image" src="https://user-images.githubusercontent.com/30142575/174765317-6186e37f-1335-471a-a623-06f1ecf22850.png">

+ help

<img width="510" alt="image" src="https://user-images.githubusercontent.com/30142575/174765336-546449ee-25a7-4844-9d9f-396b685ab806.png">

+ exit

<img width="311" alt="image" src="https://user-images.githubusercontent.com/30142575/174765352-1c95c529-2c5c-4209-b1a2-53caf19edf5f.png">

+ log.txt
  + 디몬프로세스 확인

<img width="510" alt="image" src="https://user-images.githubusercontent.com/30142575/174765425-2b4cbfaf-41e2-48e0-8c4d-7233828aa801.png">

  + 생성, 삭제 ,수정 실행 (1_1.c와 1.c는 원래있던 파일)

<img width="320" alt="image" src="https://user-images.githubusercontent.com/30142575/174765510-00362e7d-bb35-4a7e-9ca8-40814e1aab6b.png">
  + log.txt 확인

<img width="405" alt="image" src="https://user-images.githubusercontent.com/30142575/174765567-61354edf-82b4-4281-9dda-54ef9a9357db.png">

+ 크기 초과시 info파일 삭제

<img width="371" alt="image" src="https://user-images.githubusercontent.com/30142575/174765626-0f7ba6cc-f627-45ee-ba40-94083ea81953.png">
<img width="228" alt="image" src="https://user-images.githubusercontent.com/30142575/174765642-5a2edaf1-11e4-40d9-8b26-2ee432ddef2e.png">

+ 삭제 확인

<img width="485" alt="image" src="https://user-images.githubusercontent.com/30142575/174765717-25750c4a-9ff9-408d-bd9b-2bace64a27ed.png">

+ files에서도 삭제 되었는지 확인
<img width="510" alt="image" src="https://user-images.githubusercontent.com/30142575/174765787-855e4718-eb57-4a52-985b-c8f0f94cdc5a.png">
