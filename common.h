#ifndef COMMON_H
#define COMMON_H

/**
  * @brief 防窄化类型强转
  * @param 源类型
  * @return 目标类型
  * @auther JSCao
  * @date   2019-03-23
  */
template<class Target, class Source>
Target narrow_cast(Source v)
{
    auto r = static_cast<Target>(v);
    if (static_cast<Source>(r) != v) {
        throw std::runtime_error("narrow_cast<> failed!");
    }
    return r;
}

#endif // COMMON_H
