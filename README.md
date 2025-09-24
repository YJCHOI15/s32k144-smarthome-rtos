# S32K144 기반 RTOS 스마트홈 제어 시스템

본 프로젝트는 NXP S32K144 MCU를 사용하여 FreeRTOS 기반으로 구현된 스마트홈 제어 시스템입니다. 다양한 센서로부터 환경 데이터를 수집하고, 사용자의 입력과 설정된 로직에 따라 각종 액추에이터를 제어합니다. 특히 CAN 통신을 통해 외부 시스템에서의 원격 모니터링 및 제어 기능을 지원하여 확장성을 확보했습니다. 

개발 전 과정은 V-모델 개발 프로세스를 따라 체계적으로 진행되었습니다. [docs](https://github.com/YJCHOI15/s32k144-smarthome-rtos/tree/main/docs)에서 확인하실 수 있습니다.

<img width="1279" height="530" alt="image" src="https://github.com/user-attachments/assets/a0f75bb5-8f05-4e30-b317-147868967b6e" />

---

## 주요 기능

- 3가지 동작 모드

  - 모니터링 모드 (Monitoring): 온/습도, 조도 센서 값을 기반으로 LED PWM과 DC Motor을 자동 제어하여 최적의 환경을 유지합니다.

  - 수동 모드 (Manual): 사용자가 버튼과 가변저항을 이용해 Servo Motor, Step Motor, Relay, LED PWM 등을 직접 제어합니다.

  - 보안 모드 (Security): 초음파 센서로 움직임을 감지하여 침입 발생 시 경고등과 부저를 활성화합니다.

- 실시간 데이터 표시: OLED와 FND를 통해 현재 시스템 모드, 제어 대상, 온/습도, 밝기 정보를 실시간으로 표시합니다.

- 원격 제어 및 모니터링 (CAN): CAN 통신을 통해 시스템의 모든 상태를 주기적으로 외부에 전송하며, 외부 명령을 수신하여 모드 변경, 장치 제어, 보안 경고 해제 등을 수행합니다.

- RTOS 기반의 멀티태스킹: FreeRTOS를 사용하여 센서 데이터 수집, 사용자 입력 처리, 디스플레이 업데이트, 통신 등 여러 기능을 독립적인 Task로 분리하여 안정적이고 실시간성 높은 동작을 보장합니다.

---

## 시스템 아키텍처 (System Architecture)

### 하드웨어 구성

시스템은 S32K144를 중심으로 다양한 센서와 액추에이터로 구성됩니다.

<img width="1361" height="1058" alt="image" src="https://github.com/user-attachments/assets/88f4f0ba-1d04-4ace-b8a1-820acbd26ed7" />

# 소프트웨어 아키텍처

소프트웨어는 RTOS 기반의 계층적 구조로 설계되어 안정성과 확장성을 높였습니다. 각 기능은 독립적인 Task로 동작하며, Message Queue, Semaphore, Event Group을 통해 안전하게 통신합니다.

<img width="1089" height="842" alt="image" src="https://github.com/user-attachments/assets/3b06ba26-6d30-48f4-90cb-88883e2b1723" />

- Driver Layer: MCU의 주변장치(ADC, FTM, CAN 등)를 직접 제어하는 저수준 드라이버.

- HAL (Hardware Abstraction Layer): 드라이버를 추상화하여 SHH_DoorLock_Open()과 같이 직관적인 API를 제공.

- Middleware Layer: 시스템 전역에서 사용되는 데이터 구조체(command_msg_t 등)와 RTOS 객체를 정의.

- Task (Application) Layer: HAL API를 사용하여 시스템의 실제 동작 로직을 구현하는 최상위 계층.

  <img width="1096" height="756" alt="image" src="https://github.com/user-attachments/assets/fa4f35ac-8d22-483d-8a7b-0999903964e1" />

---

## 테스트 및 검증 (Testing & Validation)

본 프로젝트는 안정성 확보를 위해 체계적인 4단계 테스트를 거쳤습니다.

Unit Testing: HAL 함수들이 개별적으로 정확히 동작하는지 검증. (DHT11 타이밍, PWM 자원 충돌 문제 해결)

Integration Testing: RTOS Task들이 Queue, Mutex 등을 통해 유기적으로 협력하는지 검증. (스택 오버플로우로 인한 데이터 오염 문제 해결)

System Testing: 실제 사용자 시나리오 기반으로 모든 기능이 요구사항대로 동작하는지 검증. (RTOS Heap/Stack 메모리 부족 문제 해결)

Acceptance Testing: 최종 완성품의 동작을 시연하고, 로직 애널라이저로 내부 신호의 안정성을 최종 확인.

<img width="1244" height="591" alt="image" src="https://github.com/user-attachments/assets/bb03d70d-0c96-418f-a53a-dca855ac013b" />
▲ 최종 시스템 동작 신호 파형

<BR><BR>

<img width="919" height="901" alt="image" src="https://github.com/user-attachments/assets/c13025f4-663d-4d9e-9c21-7909f1bb0b1b" /><BR>
▲ 매뉴얼 모드일 때 OLED, FND 및 LED 표시 모습
