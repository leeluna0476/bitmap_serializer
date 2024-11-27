# bitmap_serializer
간단한 bitmap serializer.

유저가 지정한 색상과 파일명으로 .bmp 파일을 생성한다.

## User Interface
- 이미지 크기 선택. (최소: )
- palette에서 선택한 3가지 색상을 각각 1, 2, 3으로 입력.
- 배경색 선택. (흑/백)
- 방향키로 커서를 움직이면서 그리기.

## Drawing functions
- [ ] palette 종류 설정. (1 palette = 3 colors)
- [ ] 배경색 설정. (흑/백)
- [ ] 그리기 시작.
- [ ] 전체 clear.
- [ ] 그리기 완료.

## Image creation
- 유저가 입력한 1, 2, 3을 팔레트의 각 색에 대응시켜 픽셀값 결정.
- TUI 픽셀 위치를 실제 이미지 픽셀 위치로 보정.
