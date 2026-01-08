
## 双端队列
采用双向链表实现



## 循环队列

#define N 1023
int Hash[N] = {0};

struct myqueue {
	int data[N];
	int head, rear;

	// head 头
	// rear 可插入的位置
	bool init() {
		head = rear = 0;
		return true;
	}

	int size() { return (rear - head + N) % N;}

	bool empty() {
		return size() == 0;
	}

	// 仅剩一个位置，认为满了，无法插入
	bool push(int e) {
		if ((rear + 1) % N == head) return false;

		data[rear] = e;
		rear = (rear + 1) % N;
		return true;
	}

	bool pop(int e) {
		if (head == rear) return false;

		head = (head + 1) % N;
		return true;
	}

	int front() {
		return data[head];
	}

	int back() {
		int seq = (rear - 1 + N) % N;
		return data[seq];
	}
};
