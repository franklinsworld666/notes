
## 题目

https://leetcode.cn/problems/two-sum/

## tag 
数组
字典


## soulution1

2层循环遍历，找出和等于 target 的两个元素；

时间:O(n*n)

空间:O(1)

## solution2

用一个 map 存放 <value, position>；从前往后遍历每一个元素，如果 target - 当前元素 可以找到，也就找到了答案；
如果不能找到，那么把当前元素放入 map；继续访问下一个元素；

time:O(nlogn)

space:O(n)

## code 
```python3
class Solution:
    def twoSum(self, nums: List[int], target: int) -> List[int]:
        mydict = {}
        i = 0
        while i < len(nums):
            if target - nums[i] in mydict:
                return [i, mydict[target - nums[i]]]
            else:
                mydict[nums[i]] = i
                i = i + 1
        return mydict
```

```cpp
class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        unordered_map<int, int> map;
        for (int i = 0; i < nums.size(); ++i) {
            auto it = map.find(target - nums[i]);
            if (it != map.end()) {
                return {i, it->second};
            }
            else {
                map[nums[i]] = i;
            }
        }
        return {};
    }
};
```