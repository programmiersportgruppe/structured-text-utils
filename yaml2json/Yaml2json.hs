import Control.Applicative
import qualified Data.Map as M
import Data.Ratio
import qualified Data.Text as T
import System.Console.GetOpt
import System.Environment
import System.Exit
import System.IO
import Text.JSON
import Text.StringTemplate
import Data.Yaml.Syck hiding (unpackBuf, packBuf)
import Codec.Binary.UTF8.String (encodeString, decodeString)
import qualified Data.Yaml.Syck (unpackBuf, packBuf)
import qualified Data.ByteString.Char8 as B (ByteString, readFile, filter)


main :: IO ()

identity :: String -> String
identity x = x

main = getContents >>= parseYaml >>= u

u :: YamlNode -> IO()
u x = putStr(convert(x))

convert :: YamlNode -> String
convert x = encode(yamlNodeToJSValue(x))


yamlNodeToJSValue :: YamlNode -> JSValue
yamlNodeToJSValue n =
  case n_elem n of
        EStr s   -> JSString (toJSString (unpackBuf s))
        EMap xs | all (\(k,_) -> isStrNode k) xs -> JSObject (toJSObject pairs)
                     where pairs = foldr addPair [] xs
                           addPair (k,v) acc = if isStrNode k && strFrom k == "<<"  -- hash merge, unsupported in Syck!
                                                  then case n_elem v of             -- so we do it ourselves
                                                            EMap ys -> foldr addPair acc ys
                                                            _       -> error "Tried hash merge on non-hash"
                                                  else case lookup kstr acc of
                                                             Just _  -> acc   -- overridden
                                                             Nothing -> (kstr, yamlNodeToJSValue v) : acc
                                                          where kstr = strFrom k
        EMap _  -> error "Map keys must all be strings."
        ESeq xs -> JSString (toJSString "test") --NList $ map yamlNodeToNode xs
        ENil    -> JSNull

strip :: String -> String
strip = T.unpack . T.strip . T.pack

isStrNode :: YamlNode -> Bool
isStrNode x = case n_elem x of
                   EStr _ -> True
                   _      -> False

strFrom :: YamlNode -> String
strFrom x = case n_elem x of
                 EStr z   -> unpackBuf z
                 _        -> error "expected EStr node"

type Buf = B.ByteString


unpackBuf :: Buf -> String
unpackBuf = decodeString . Data.Yaml.Syck.unpackBuf


instance ToSElem JSValue where
    toSElem JSNull = toSElem (Nothing :: Maybe String)
    toSElem (JSString s) = toSElem (fromJSString s)
    toSElem (JSBool b) = toSElem b
    toSElem (JSRational _ r)
        | denominator r == 1 = toSElem (numerator r)
        | otherwise          = toSElem (fromRational r :: Double)
    toSElem (JSArray l) = toSElem l
    toSElem (JSObject o) = toSElem . M.fromList $ fromJSObject o