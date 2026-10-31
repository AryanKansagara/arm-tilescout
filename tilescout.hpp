#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <algorithm>
namespace tile {
using Matrix=std::vector<std::int8_t>; using Output=std::vector<std::int64_t>;
enum class Layout{column_padded,block4};
struct Packed{std::size_t k,n,kp,np,offset;Layout layout;Matrix bytes;};
inline std::size_t round4(std::size_t x){return (x+3)/4*4;}
inline void shape(std::size_t m,std::size_t k,std::size_t n){if(m>64||k>64||n>64)throw std::runtime_error("dimension exceeds 64");}
inline std::size_t index(const Packed&p,std::size_t k,std::size_t n){if(p.layout==Layout::column_padded)return p.offset+n*p.kp+k;if(p.layout==Layout::block4)return p.offset+((n/4)*p.kp+k)*4+n%4;throw std::runtime_error("unknown packing layout");}
inline Packed pack(const Matrix&b,std::size_t k,std::size_t n,Layout layout,std::size_t offset){shape(0,k,n);if((layout!=Layout::column_padded&&layout!=Layout::block4)||b.size()!=k*n||offset>7)throw std::runtime_error("source shape or offset invalid");Packed p{k,n,round4(k),round4(n),offset,layout,Matrix(offset+round4(k)*round4(n)+8,0)};for(std::size_t i=0;i<offset;++i)p.bytes[i]=99;for(std::size_t i=p.bytes.size()-8;i<p.bytes.size();++i)p.bytes[i]=99;for(std::size_t y=0;y<k;++y)for(std::size_t x=0;x<n;++x)p.bytes[index(p,y,x)]=b[y*n+x];return p;}
inline void validate(const Packed&p,std::size_t k,std::size_t n){shape(0,k,n);if(p.k!=k||p.n!=n||p.kp!=round4(k)||p.np!=round4(n)||p.offset>7||(p.layout!=Layout::column_padded&&p.layout!=Layout::block4)||p.bytes.size()!=p.offset+p.kp*p.np+8)throw std::runtime_error("incompatible packing metadata");for(std::size_t y=0;y<p.kp;++y)for(std::size_t x=0;x<p.np;++x)if((y>=k||x>=n)&&p.bytes[index(p,y,x)]!=0)throw std::runtime_error("nonzero packed padding");for(std::size_t i=0;i<p.offset;++i)if(p.bytes[i]!=99)throw std::runtime_error("leading guard changed");for(std::size_t i=p.bytes.size()-8;i<p.bytes.size();++i)if(p.bytes[i]!=99)throw std::runtime_error("trailing guard changed");}
struct Range{std::size_t begin,end;};
inline void partitions(std::size_t m,const std::vector<Range>&ranges){std::vector<unsigned> seen(m,0);for(auto r:ranges){if(r.begin>r.end||r.end>m)throw std::runtime_error("partition outside rows");for(auto i=r.begin;i<r.end;++i)if(++seen[i]!=1)throw std::runtime_error("partition overlaps");}for(auto x:seen)if(x!=1)throw std::runtime_error("partition leaves a hole");}
inline Output reference(const Matrix&a,const Matrix&b,std::size_t m,std::size_t k,std::size_t n){shape(m,k,n);if(a.size()!=m*k||b.size()!=k*n)throw std::runtime_error("reference input shape");Output c(m*n,0);for(std::size_t row=0;row<m;++row)for(std::size_t col=0;col<n;++col)for(std::size_t inner=0;inner<k;++inner)c[row*n+col]+=static_cast<std::int64_t>(a[row*k+inner])*b[inner*n+col];return c;}
inline Output run(const Matrix&a,const Packed&p,std::size_t m,std::size_t k,std::size_t n,const std::vector<Range>&ranges){shape(m,k,n);if(a.size()!=m*k)throw std::runtime_error("left shape");validate(p,k,n);partitions(m,ranges);Output c(m*n,0);for(auto r:ranges)for(auto row=r.begin;row<r.end;++row)for(std::size_t base=0;base<p.np;base+=4)for(std::size_t inner=0;inner<p.kp;++inner)for(std::size_t lane=0;lane<4;++lane){auto col=base+lane;if(col<n&&inner<k)c[row*n+col]+=static_cast<std::int64_t>(a[row*k+inner])*p.bytes[index(p,inner,col)];}return c;}
inline std::string dispatch(bool aarch64,bool dotprod,Layout layout){return aarch64&&dotprod&&layout==Layout::block4?"eligible_portable_block4":"scalar_fallback";}
struct Demo{std::size_t cases=0,matches=0,tail_defects=0;};
inline Demo evaluate(){Demo d;for(std::size_t m:{0,1,3,5})for(std::size_t k:{0,1,3,4,7})for(std::size_t n:{0,1,3,4,7})for(auto l:{Layout::column_padded,Layout::block4})for(std::size_t off:{0,1,7}){Matrix a(m*k),b(k*n);for(std::size_t i=0;i<a.size();++i)a[i]=static_cast<std::int8_t>(static_cast<int>(i%13)-6);for(std::size_t i=0;i<b.size();++i)b[i]=static_cast<std::int8_t>(static_cast<int>(i%11)-5);auto expected=reference(a,b,m,k,n);auto actual=run(a,pack(b,k,n,l,off),m,k,n,{{0,m/2},{m/2,m}});++d.cases;if(actual==expected)++d.matches;auto faulty=actual;if(n%4)for(std::size_t row=0;row<m;++row)faulty[row*n+n-1]=0;if(faulty!=expected)++d.tail_defects;}return d;}
}
