
```cpp
#include <iostream>
#include <optional>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 安全读取 string
std::optional<std::string> getString(
    const json& obj,
    const std::string& key)
{
    // 1. key 不存在
    if (!obj.contains(key))
    {
        return std::nullopt;
    }

    // 2. key 存在，但是值为 null
    if (obj[key].is_null())
    {
        return std::nullopt;
    }

    // 3. 类型不是 string
    if (!obj[key].is_string())
    {
        return std::nullopt;
    }

    return obj[key].get<std::string>();
}

int main()
{
    // ============================================================
    // 1. 创建 JSON object
    // ============================================================

    json j;

    j["Name"] = "BMC";
    j["Enabled"] = true;
    j["Port"] = 443;

    std::cout << "===== Basic object =====\n";
    std::cout << j.dump(4) << "\n\n";


    // ============================================================
    // 2. 创建嵌套 object
    // ============================================================

    j["Status"]["State"] = "Enabled";
    j["Status"]["Health"] = "OK";

    std::cout << "===== Nested object =====\n";
    std::cout << j.dump(4) << "\n\n";


    // ============================================================
    // 3. 创建 JSON array
    // ============================================================

    j["Members"] = json::array();

    j["Members"].push_back({
        {"@odata.id", "/redfish/v1/Systems/system"}
    });

    j["Members"].push_back({
        {"@odata.id", "/redfish/v1/Systems/system2"}
    });

    std::cout << "===== Array =====\n";
    std::cout << j.dump(4) << "\n\n";


    // ============================================================
    // 4. 读取一个确定存在的值
    // ============================================================

    std::string name = j["Name"].get<std::string>();
    bool enabled = j["Enabled"].get<bool>();

    std::cout << "Name    = " << name << "\n";
    std::cout << "Enabled = " << enabled << "\n\n";


    // ============================================================
    // 5. 判断 JSON 类型
    // ============================================================

    std::cout << "===== Type check =====\n";

    if (j["Name"].is_string())
    {
        std::cout << "Name is string\n";
    }

    if (j["Port"].is_number())
    {
        std::cout << "Port is number\n";
    }

    if (j["Enabled"].is_boolean())
    {
        std::cout << "Enabled is boolean\n";
    }

    if (j["Status"].is_object())
    {
        std::cout << "Status is object\n";
    }

    if (j["Members"].is_array())
    {
        std::cout << "Members is array\n";
    }

    std::cout << "\n";


    // ============================================================
    // 6. 遍历 array
    // ============================================================

    std::cout << "===== Iterate array =====\n";

    for (const auto& member : j["Members"])
    {
        std::cout << member["@odata.id"].get<std::string>() << "\n";
    }

    std::cout << "\n";


    // ============================================================
    // 7. 重点：key 不存在
    // ============================================================

    std::cout << "===== Missing value =====\n";

    if (!j.contains("Model"))
    {
        std::cout << "Model does not exist\n";
    }


    // ============================================================
    // 8. value()：不存在时给默认值
    // ============================================================

    std::string model = j.value("Model", "Unknown");

    std::cout << "Model = " << model << "\n\n";


    // ============================================================
    // 9. JSON null
    // ============================================================

    j["SerialNumber"] = nullptr;

    if (j["SerialNumber"].is_null())
    {
        std::cout << "SerialNumber exists, but its value is null\n";
    }

    std::cout << "\n";


    // ============================================================
    // 10. std::optional：推荐方式
    // ============================================================

    std::optional<std::string> modelOpt = getString(j, "Model");

    if (modelOpt)
    {
        // optional 重载了 *，取出对象中的值
        std::cout << "Model = " << *modelOpt << "\n";
    }
    else
    {
        std::cout << "Model has no valid value\n";
    }

    std::optional<std::string> nameOpt = getString(j, "Name");

    if (nameOpt)
    {
        std::cout << "Name = " << *nameOpt << "\n";
    }


    // ============================================================
    // 11. 遍历整个 object
    // ============================================================

    std::cout << "\n===== Iterate object =====\n";

    for (auto it = j.begin(); it != j.end(); ++it)
    {
        std::cout << "key = " << it.key()
                  << ", value = " << it.value()
                  << "\n";
    }

    return 0;
}
```