// ���� ifdef ����� DLL���� ���������ϴ� �۾��� ���� �� �ִ� ��ũ�θ� ����� 
// ǥ�� ����Դϴ�. �� DLL�� ��� �ִ� ������ ��� ����ٿ� ���ǵ� _EXPORTS ��ȣ��
// �����ϵǸ�, �ٸ� ������Ʈ������ �� ��ȣ�� ������ �� �����ϴ�.
// �̷��� �ϸ� �ҽ� ���Ͽ� �� ������ ��� �ִ� �ٸ� ��� ������Ʈ������ 
// MESSAGESYP_API �Լ��� DLL���� �������� ������ ����, �� DLL��
// �� DLL�� �ش� ��ũ�η� ���ǵ� ��ȣ�� ���������� ������ ���ϴ�.
#ifdef MESSAGESYP_EXPORTS
#define MESSAGESYP_API __declspec(dllexport)
#else
#define MESSAGESYP_API __declspec(dllimport)
#endif

// �� Ŭ������ MessageSYP.dll���� ������ ���Դϴ�.
class MESSAGESYP_API CMessageSYP {
public:
	CMessageSYP(void);
	// TODO: ���⿡ �޼��带 �߰��մϴ�.
};

extern MESSAGESYP_API int nMessageSYP;

MESSAGESYP_API int fnMessageSYP(void);
