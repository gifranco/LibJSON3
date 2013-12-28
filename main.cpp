#include "json/json.h"
#include <algorithm> // sort
#include <stdio.h>

#if defined(_MSC_VER)  &&  _MSC_VER >= 1310
# pragma warning( disable: 4996 )     // disable fopen deprecation warning
#endif

static std::string
readInputTestFile( const char *path )
{
   FILE *file = fopen( path, "rb" );
   if ( !file )
      return std::string("");
   fseek( file, 0, SEEK_END );
   long size = ftell( file );
   fseek( file, 0, SEEK_SET );
   std::string text;
   char *buffer = new char[size+1];
   buffer[size] = 0;
   if ( fread( buffer, 1, size, file ) == (unsigned long)size )
      text = buffer;
   fclose( file );
   delete[] buffer;
   return text;
}


static void
printValueTree( Json::Value &value, std::vector<std::string> &js, const std::string &path = "." )
{
   char str[100];

   switch ( value.type() )
   {
	   case Json::nullValue:
		  //fprintf( fout, "%s=null\n", path.c_str() );
		  js.push_back(path);
		  break;
	   case Json::intValue:
		  //fprintf( fout, "%s=%d\n", path.c_str(), value.asInt() );
		  sprintf( str, "%s=%d\n", path.c_str(), value.asInt() );
		  js.push_back(str);
		  break;
	   case Json::uintValue:
		  //fprintf( fout, "%s=%u\n", path.c_str(), value.asUInt() );
		  sprintf( str, "%s=%u\n", path.c_str(), value.asUInt() );
		  js.push_back(str);
		  break;
	   case Json::realValue:
		  //fprintf( fout, "%s=%.16g\n", path.c_str(), value.asDouble() );
		  sprintf( str, "%s=%.16g\n", path.c_str(), value.asDouble() );
		  js.push_back(str);
		   break;
	   case Json::stringValue:
		  //fprintf( fout, "%s=\"%s\"\n", path.c_str(), value.asString().c_str() );
		  sprintf( str, "%s=\"%s\"\n", path.c_str(), value.asString().c_str() );
		  js.push_back(str);
		  break;
	   case Json::booleanValue:
		  //fprintf( fout, "%s=%s\n", path.c_str(), value.asBool() ? "true" : "false" );
		  sprintf( str, "%s=%s\n", path.c_str(), value.asBool() ? "true" : "false" );
		  js.push_back(str);
		 break;
	   case Json::arrayValue:
		  {
			 //fprintf( fout, "%s=[]\n", path.c_str() );
	//		 sprintf( str, "%s=[]\n", path.c_str() );
	//		 js.push_back(str);
			 int size = value.size();
			 for ( int index =0; index < size; ++index )
			 {
				static char buffer[16];
				sprintf( buffer, "[%d]", index );
				printValueTree( value[index], js, path + buffer );
			 }
		  }
		  break;
	   case Json::objectValue:
		  {
	//         //fprintf( fout, "%s={}\n", path.c_str() );
			 //fprintf( fout, "-----\n", path.c_str() );
			 Json::Value::Members members( value.getMemberNames() );
			 //std::sort( members.begin(), members.end() );
			 std::string suffix = *(path.end()-1) == '.' ? "" : ".";
			 for ( Json::Value::Members::iterator it = members.begin();
				   it != members.end();
				   ++it )
			 {
				const std::string &name = *it;
				printValueTree( value[name], js, path + suffix + name );
			 }
		  }
		  break;
	   default:
		  break;
   }
}

static void
printValueTree( FILE *fout, Json::Value &value, const std::string &path = "." )
{
   switch ( value.type() )
   {
   case Json::nullValue:
      fprintf( fout, "%s=null\n", path.c_str() );
      break;
   case Json::intValue:
      fprintf( fout, "%s=%d\n", path.c_str(), value.asInt() );
      break;
   case Json::uintValue:
      fprintf( fout, "%s=%u\n", path.c_str(), value.asUInt() );
      break;
   case Json::realValue:
      fprintf( fout, "%s=%.16g\n", path.c_str(), value.asDouble() );
      break;
   case Json::stringValue:
      fprintf( fout, "%s=\"%s\"\n", path.c_str(), value.asString().c_str() );
      break;
   case Json::booleanValue:
      fprintf( fout, "%s=%s\n", path.c_str(), value.asBool() ? "true" : "false" );
      break;
   case Json::arrayValue:
      {
         fprintf( fout, "%s=[]\n", path.c_str() );
         int size = value.size();
         for ( int index =0; index < size; ++index )
         {
            static char buffer[16];
            sprintf( buffer, "[%d]", index );
            printValueTree( fout, value[index], path + buffer );
         }
      }
      break;
   case Json::objectValue:
      {
         fprintf( fout, "%s={}\n", path.c_str() );
         Json::Value::Members members( value.getMemberNames() );
         std::sort( members.begin(), members.end() );
         std::string suffix = *(path.end()-1) == '.' ? "" : ".";
         for ( Json::Value::Members::iterator it = members.begin(); 
               it != members.end(); 
               ++it )
         {
            const std::string &name = *it;
            printValueTree( fout, value[name], path + suffix + name );
         }
      }
      break;
   default:
      break;
   }
}


static int
parseAndSaveValueTree( const std::string &input, 
                       const std::string &actual,
                       const std::string &kind,
                       Json::Value &root,
                       const Json::Features &features,
                       bool parseOnly )
{
   Json::Reader reader( features );
   bool parsingSuccessful = reader.parse( input, root );
   if ( !parsingSuccessful )
   {
      printf( "Failed to parse %s file: \n%s\n", 
              kind.c_str(),
              reader.getFormatedErrorMessages().c_str() );
      return 1;
   }

   if ( !parseOnly )
   {
      FILE *factual = fopen( actual.c_str(), "wt" );
      if ( !factual )
      {
         printf( "Failed to create %s actual file.\n", kind.c_str() );
         return 2;
      }
      printValueTree( factual, root );
      fclose( factual );
   }
   return 0;
}


static int
rewriteValueTree( const std::string &rewritePath, 
                  const Json::Value &root, 
                  std::string &rewrite )
{
   //Json::FastWriter writer;
   //writer.enableYAMLCompatibility();
   Json::StyledWriter writer;
   rewrite = writer.write( root );
   FILE *fout = fopen( rewritePath.c_str(), "wt" );
   if ( !fout )
   {
      printf( "Failed to create rewrite file: %s\n", rewritePath.c_str() );
      return 2;
   }
   fprintf( fout, "%s\n", rewrite.c_str() );
   fclose( fout );
   return 0;
}


static std::string
removeSuffix( const std::string &path, 
              const std::string &extension )
{
   if ( extension.length() >= path.length() )
      return std::string("");
   std::string suffix = path.substr( path.length() - extension.length() );
   if ( suffix != extension )
      return std::string("");
   return path.substr( 0, path.length() - extension.length() );
}

static int 
printUsage( const char *argv[] )
{
   printf( "Usage: %s [--strict] input-json-file", argv[0] );
   return 3;
}


int
parseCommandLine( int argc, const char *argv[], 
                  Json::Features &features, std::string &path,
                  bool &parseOnly )
{
   parseOnly = false;
   if ( argc < 2 )
   {
      return printUsage( argv );
   }

   int index = 1;
   if ( std::string(argv[1]) == "--json-checker" )
   {
      features = Json::Features::strictMode();
      parseOnly = true;
      ++index;
   }

   if ( index == argc  ||  index + 1 < argc )
   {
      return printUsage( argv );
   }

   path = argv[index];
   return 0;
}

void GetData( std::string str, std::string  &var, std::string &dat )
{
	int pos, len;
	std::string s = str;

	while((pos=s.find("\n")) > -1)
	{
		s.replace(pos, 1,"" );
	}

	while((pos=s.find("\"")) > -1)
	{
		s.replace(pos, 1,"" );
	}

	pos = s.find('=');
	len = s.length();

	dat = s.substr(pos+1,len-pos);

	s = s.substr(0,pos);

	while((pos=s.find(".")) > -1)
	{
		s.replace(pos, 1,"" );
	}
	var = s;

}


int main( int argc, const char *argv[] )
{
   bool 					parseOnly;
   std::string 				path;
   Json::Features 			features;
   Json::Reader				reader(features);
   std::string 				input;
   std::string 				basePath;
   std::string 				actualPath;
   std::string 				rewritePath;
   std::string 				rewriteActualPath;
   Json::Value 				root;
   std::vector<std::string>	jsonData;


   int exitCode = parseCommandLine( argc, argv, features, path, parseOnly );
   if ( exitCode != 0 )
   {
      return exitCode;
   }

   /*std::string */input = readInputTestFile( path.c_str() );
   if ( input.empty() )
   {
      printf( "Failed to read input or empty input: %s\n", path.c_str() );
      return 3;
   }

   printf("intput: %s\n", input.c_str());

	std::string  s, var, dat;

   if(reader.parse(input,root))
   {
	   printValueTree(root, jsonData);

		for( int i=0; i<jsonData.size(); i++)
		{
			s = jsonData[i];

			GetData( s, var, dat );

			printf("%s : %s\n", var.c_str() , dat.c_str());
		}
   }
#if 0

   /*std::string */basePath = removeSuffix( argv[1], ".json" );
   if ( !parseOnly  &&  basePath.empty() )
   {
      printf( "Bad input path. Path does not end with '.expected':\n%s\n", path.c_str() );
      return 3;
   }

   actualPath        = basePath + ".actual";
   rewritePath       = basePath + ".rewrite";
   rewriteActualPath = basePath + ".actual-rewrite";


   exitCode = parseAndSaveValueTree( input, actualPath, "input", root, features, parseOnly );
   if ( exitCode == 0  &&  !parseOnly )
   {
      std::string rewrite;
      exitCode = rewriteValueTree( rewritePath, root, rewrite );
      if ( exitCode == 0 )
      {
         Json::Value rewriteRoot;
         exitCode = parseAndSaveValueTree( rewrite, rewriteActualPath, 
            "rewrite", rewriteRoot, features, parseOnly );
      }
   }
#endif
   return exitCode;
}

