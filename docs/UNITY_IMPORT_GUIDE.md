# Unity 임포트 가이드

## 생성된 JSON 파일 임포트하기

### 방법 1: JSON Importer 사용 (권장)

1. Unity Editor 열기
2. 메뉴: `Tools > Rundee > Item Factory > JSON Importer`
3. 각 JSON 파일을 순서대로 임포트:
   - **Item Type**: Food → `TestOutput/items_food.json` 선택 → "Import Selected JSON"
   - **Item Type**: Drink → `TestOutput/items_drink.json` 선택 → "Import Selected JSON"
   - **Item Type**: Material → `TestOutput/items_material.json` 선택 → "Import Selected JSON"
   - **Item Type**: Weapon → `TestOutput/items_weapon.json` 선택 → "Import Selected JSON"
   - **Item Type**: Weapon Component → `TestOutput/items_weaponcomponent.json` 선택 → "Import Selected JSON"
   - **Item Type**: Ammo → `TestOutput/items_ammo.json` 선택 → "Import Selected JSON"
   - **Item Type**: Armor → `TestOutput/items_armor.json` 선택 → "Import Selected JSON" (Armor 지원 확인 필요)
   - **Item Type**: Clothing → `TestOutput/items_clothing.json` 선택 → "Import Selected JSON" (Clothing 지원 확인 필요)

### 방법 2: Item Factory Window 사용

1. Unity Editor 열기
2. 메뉴: `Tools > Rundee > Item Factory > Item Factory Window`
3. "JSON Importer" 섹션 사용 (위와 동일)

### 임포트 후 확인

1. **Item Manager**에서 확인:
   - `Tools > Rundee > Item Factory > Item Manager`
   - 각 타입별로 50개씩 생성되었는지 확인

2. **프로젝트 창**에서 확인:
   - `Assets/Resources/RundeeItemFactory/`
   - 각 타입별 폴더에 ScriptableObject들이 생성되었는지 확인

## 생성된 파일 위치

### JSON 파일
- 위치: `TestOutput/items_*.json`
- 각 타입별로 50개씩 생성됨

### ScriptableObject (임포트 후)
- 위치: `Assets/Resources/RundeeItemFactory/[Type]Items/`
- 예: `Assets/Resources/RundeeItemFactory/FoodItems/`

## 문제 해결

### 임포트 실패 시
1. JSON 파일이 올바른 형식인지 확인
2. Unity Console에서 에러 메시지 확인
3. ItemImporter.cs의 로그 확인

### 아이템이 보이지 않을 때
1. Item Manager에서 검색
2. Resources 폴더 확인
3. Unity Editor 재시작

## 다음 단계

임포트가 완료되면:
1. Item Manager에서 아이템 검색 및 관리
2. ItemDatabase를 사용하여 런타임에서 아이템 로드
3. 게임에서 아이템 사용



