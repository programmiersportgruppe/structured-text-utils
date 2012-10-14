import Control.Applicative
import Text.JSON
import Data.Yaml.Syck hiding (unpackBuf, packBuf)
import Codec.Binary.UTF8.String (decodeString)
import qualified Data.Yaml.Syck (unpackBuf)
import qualified Data.ByteString.Char8 as B (ByteString)


main :: IO ()

main = getContents >>= parseYaml >>=  putStr .  cr . encode . yamlNodeToJSValue

cr :: String -> String
cr s = s ++ "\n"

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
        ESeq xs -> JSArray (map yamlNodeToJSValue xs)
        ENil    -> JSNull

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

