#ifndef __VERSION_BASE_H__
#define __VERSION_BASE_H__

namespace VERSION
{
	enum
	{
		E_MAJOR = 0,
		E_MINOR = 1,
		E_REVISION = 2,
		E_LENGTH
	};
}


class CVersion_base
{
public:
    CVersion_base(){}
			
		CVersion_base(uint32_t v0, uint32_t v1, uint32_t v2)
		{
			rawVersion[0] = v0;
			rawVersion[1] = v1;
			rawVersion[2] = v2;
		}
		
		int32_t getMajor() const {return rawVersion[VERSION::E_MAJOR];}
    int32_t getMinor() const {return rawVersion[VERSION::E_MINOR];}
    int32_t getRevision() const {return rawVersion[VERSION::E_REVISION];}
		
		int32_t ver(uint8_t idx) const
		{
			if(idx >= VERSION::E_LENGTH)
				return -1;
			else 
				return rawVersion[idx];
		}
		
		int writeVer(uint8_t idx, int32_t val)
		{
			if(idx >= VERSION::E_LENGTH)
				return -1;
			else 
				rawVersion[idx] = val;
			
			return 0;
		}

		bool operator==(const CVersion_base& vref) const
		{
			for(int i = 0; i < VERSION::E_LENGTH; i++)
				if(ver(i) != vref.ver(i))
					return false;
			
			return true;
		}
		
		bool operator>=(const CVersion_base& vref) const
		{
			for(int i = 0; i < VERSION::E_LENGTH; i++)
				if(ver(i) < vref.ver(i))
					return false;
			
			return true;
		}
		
		bool operator<=(const CVersion_base& vref) const
		{
			for(int i = 0; i > VERSION::E_LENGTH; i++)
				if(ver(i) > vref.ver(i))
					return false;
			
			return true;
		}
		
		bool operator>(const CVersion_base& vref) const
		{
			return !(*this <= vref);
		}
		
		bool operator<(const CVersion_base& vref) const
		{
			return !(*this >= vref);
		}

private:
		int32_t rawVersion[VERSION::E_LENGTH];
};
#endif
//end of file
