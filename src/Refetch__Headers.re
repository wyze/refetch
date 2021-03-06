open Rebase;

module Cookie = Refetch__Cookie;
module Mime = Refetch__Mime;
module Utils = Refetch__Utils;

type authorizationScheme = [
  | `Basic(string, string)
  | `Bearer(string)
];

type cacheDirective = [
  | `NoCache
  | `NoStore
  | `NoTransform
  | `OnlyIfCached
  | `MaxAge(int)
  | `MaxStale(int)
  | `MinFresh(int)
  | `MustRevalidate
  | `Public
  | `Private
  | `ProxyRevalidate
  | `SMaxAge(int)
  | `Other(string, option(string))
];

type charset = [
  | `US_ASCII
  | `ISO8859_1
  | `ISO8859_2
  | `ISO8859_3
  | `ISO8859_4
  | `ISO8859_5
  | `ISO8859_6
  | `ISO8859_7
  | `ISO8859_8
  | `ISO8859_9
  | `ISO8859_10
  | `Shift_JIS
  | `EUC_JP
  | `ISO2022_KR
  | `EUC_KR
  | `ISO2022_JP
  | `ISO2022_JP_2
  | `ISO8859_6_E
  | `ISO8859_6_I
  | `ISO8859_8_E
  | `ISO8859_8_I
  | `GB2312
  | `Big5
  | `KOI8_R
  | `Other(string)
];

type languageTag; /* https://docs.rs/hyper/0.11.6/hyper/header/struct.LanguageTag.html */

type connectionOption =
  | KeepAlive
  | Close
  | ConnectionHeader(string);

type contentRangeSpec =
  | Bytes(option((int, int)), option(int))
  | Unregistered(string, string);

type dispositionType = [
  | `Inline
  | `Attachment
  | `Other(string)
];

type dispositionParam = [
  /*| `FilenameStar(charset, option(languageTag), string)*/
  | `Filename(string)
  | `Other(string, string)
];

type encoding = [
  | `Chunked
  | `Brotli
  | `Gzip
  | `Deflate
  | `Compress
  | `Idenitty
  | `Trailers
  | `Other(string)
];

type entityTag;

type httpDate;

type link;

type method = [
  | `GET
  | `HEAD
  | `POST
  | `PUT
  | `DELETE
  | `CONNECT
  | `OPTIONS
  | `TRACE
  | `PATCH
  | `OtherMethod(string)
];

type preference = [
  | `RespondAsync
  | `ReturnRepresentation
  | `ReturnMinimal
  | `HandlingStrict
  | `HandlingLenient
  | `Wait(int)
  | `Other(string, string, list((string, string)))
];

type protocolName = [
  | `HTTP
  | `TLS
  | `WebSocket
  | `H2C
  | `Other(string)
];

type protocol = {
  name: protocolName,
  version: option(string)
};

type quality = int;
type qualityItem('a) = QualityItem(quality, 'a);

type rangeUnit = [
  | `Bytes
  | `None
  | `Other(string)
];

type uri;

type entity = [
  | `ContentDisposition(dispositionType, list(dispositionParam))
  | `ContentEncoding(list(encoding))
  | `ContentLanguage(list(qualityItem(languageTag)))
  | `ContentLength(int)
  | `ContentLocation(string)
  | `ContentRange(contentRangeSpec)
  | `ContentType(Mime.t)
  | `Expires(httpDate)
  | `LastModified(httpDate)
];

type t = [
  | entity
  | `Accept(list(qualityItem(Mime.t)))
  | `AcceptCharset(list(qualityItem(charset)))
  | `AcceptEncoding(list(qualityItem(encoding)))
  | `AcceptLanguage(list(qualityItem(languageTag)))
  | `AcceptRanges(list(rangeUnit))
  | `AccessControlAllowCredentials
  | `AccessControlAllowHeaders(list(string))
  | `AccessControlAllowmethods(list(method))
  | `AccessControlExposeHeaders(list(string))
  | `AccessControlMaxAge(int)
  | `AccessControlRequestHeaders(list(string))
  | `AccessControlRequestMethod(method)
  | `Allow(list(method))
  | `Authorization(authorizationScheme)
  | `CacheControl(list(cacheDirective))
  | `Connection(list(connectionOption))
  | `Cookie(Cookie.t) 
  | `Date(httpDate)
  | `ETag(entityTag)
  | `From(string)
  | `Host(string, option(int))
  | `IfModifiedSince(httpDate)
  | `IfUnmodifiedSince(httpDate)
  | `LastEventId(string)
  | `Link(link)
  | `Location(uri)
  | `Origin(string, string, option(int))
  | `Prefer(list(preference))
  | `PreferenceApplied(list(preference))
  | `Referer(uri)
  | `Server(string)
  | `SetCookie(list(string))
  | `StrictTransportSecurity(bool, int)
  | `TE(list(qualityItem(encoding)))
  | `TransferEncoding(list(encoding))
  | `Upgrade(list(protocol))
  | `UserAgent(string)
  | `Warning(int, string, string, option(httpDate))
  | `Raw(string, string)
];

let _pairifyHeader = 
  fun | `Authorization(scheme) => {
        let value = switch (scheme) {
        
          | `Basic(username, password) =>
            let encoded = {j|$username:$password|j} |> Utils.btoa;
            {j|Basic $encoded|j}

          | `Bearer(token) =>
            {j|Bearer $token|j}
        };

        ("Authorization", value)
      }

      | `ContentDisposition(typ, params) => {
        let typ =
          switch typ {
          | `Inline => "inline"
          | `Attachment => "attachment"
          | `Other(typ) => typ
          };

        let value =
          params |> List.map (
                    fun | `Filename(filename) => {j|filename="$filename"|j}
                        | `Other(key, value) => {j|$key="$value"|j})
                 |> List.reduce((acc, p) => {j|$acc; $p|j}, typ);

        ("Content-Disposition", value)
      }

      | `ContentType(mime) =>
        ("Content-Type", Mime.toString(mime))

      | `ContentLength(length) =>
        ("Content-Length", string_of_int(length))
        
      | `Raw(name, value) => (name, value)
      | _ => failwith("TODO");
let _pairifyHeader: [< t] => _ = (h) =>
  _pairifyHeader(h :> t);

let _stringifyPair = ((name, value)) =>
  {j|$name: $value|j};

let _stringifyHeader = (header) =>
  header |> _pairifyHeader |> _stringifyPair;

let _encode = (headers) =>
  headers |> List.map(_pairifyHeader)
          |> Js.Dict.fromList
          |> Obj.magic
          |> Fetch.HeadersInit.make;
